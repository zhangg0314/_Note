# gatherexe 进程分析笔记

> 分析范围: 整体架构、IPC 通信联动、数据流向。不涉及内部实现细节。

---

## 1. 进程定位

`gatherexe` 是 SG698 集中器的**抄表执行引擎**。每个物理通道（RS485 端口或载波模块）启动一个独立实例。

**核心职责:**
- 从 `GuageFile`（持久任务队列）读取 `gathermng` 写入的抄表任务实例
- 按优先级状态机调度执行：透传 > 重要任务 > 广播任务 > 定时任务
- 构建协议帧（DL/T 698.45 / DL/T 645-2007 / dot42）
- 通过 RS485 串口或 UDP+载波模块发送帧到电表
- 解析响应、写库（`dlmsdb`）

---

## 2. IPC 全景

```
gathermng (任务调度)
    │
    ├── GuageFile (GDBM 持久队列) ──→ gatherexe 读取实例
    │
    ├── MSG_MTR_TASK_UPDATE (System V 消息队列) ──→ gatherexe 实时通知
    │
    └── g_topRunPri (共享内存) ──→ 全局优先级传递

gatherexe ──── SHM_TERM_DATA (共享内存) ────→ parser / infoproc / eventproc
```

| IPC 机制 | 方向 | 数据结构 | 用途 |
|----------|------|---------|------|
| GuageFile | gathermng → gatherexe | `oopinstance_t` | 持久任务队列 |
| MSG_MTR_TASK_UPDATE | gathermng → gatherexe | `char[]` | 新任务到达通知 |
| g_topRunPri | gathermng → gatherexe | `uint8_t` | 全局最高优先级 |
| SHM_TERM_DATA | gatherexe → 其他进程 | 实时数据/参数 | 抄表结果、PLC 信息 |

---

## 3. 架构：非抢占式优先级状态机

### 3.1 状态流转

```
INI_METERING (启动/重评估)
    │
    ├── 有透传任务? ──→ EXCUTE_TRANSTASK  (优先级最高)
    │                         ↓ 全部完成
    ├── 有重要任务? ──→ EXCUTE_IMPTASK    (重要抄表)
    │                         ↓ 全部完成
    ├── 有广播任务? ──→ EXECUTE_BROADTASK (广播)
    │                         ↓ 全部完成
    └── 有定时任务? ──→ EXECUTE_SCHEDTASK (定时巡检)
                              ↓ 全部完成
                         回到 INI_METERING
```

### 3.2 非抢占特性

状态切换只在当前状态 `Run()` 返回后发生。如果 SCHED 正在抄 200 个表，新的透传任务要等全部抄完 + INI 重新评估才能执行（可延迟数分钟到数十分钟）。

```cpp
// Handlers.cpp TaskUpdateMsgProc — 状态决策只在 INI 阶段执行
if (0 < g_transtask_nbr)          g_metering_stauts = EXCUTE_TRANSTASK;
else if (0 < g_currtask_nbr)      g_metering_stauts = EXCUTE_IMPTASK;
```

---

## 4. 数据流：TaskUpdate → Init → IMP → BROAD → SCHED

### 4.1 TaskUpdate 消息处理

```
MSG_MTR_TASK_UPDATE 到达
    → CScheduler::Update() 检测事件
    → TaskUpdateMsgProc() 读 IPC
    → 更新 g_transtask_nbr / g_currtask_nbr 计数器
    → 如果当前在 SCHED 中, 通过 CheckTaskUpdateMsg() 定时检查（非实时）
```

### 4.2 执行链 (Excute 5 步)

```
TaskEntity::Excute():
  1. PreSendProcess()  — 加载档案、解析 splitOAD、初始化协议引擎
  2. Begin()          — 记录开始时间
  3. PackAppFrame()   — 构建协议帧 (698.45 / 645-07)
  4. Send()           — RS485 串口发送 / PLC UDP 发送
  5. TranRecvFrame()  — 解析响应、写库
  6. End()            — 清理
```

---

## 5. 与 gathermng 的联动

| 步骤 | gathermng | gatherexe |
|------|-----------|-----------|
| 1 | `TaskSchdule()`: 评估定时任务配置 → 生成 `oopinstance_t` | |
| 2 | `InsertInstance()`: 写入 GuageFile | |
| 3 | `MSG_MTR_TASK_UPDATE`: 发送 IPC | → `TaskUpdateMsgProc()` 读 IPC |
| 4 | | → 从 GuageFile 查询实例 |
| 5 | | → 组帧 → 发送 → 接收 → 写库 |
| 6 | | → 更新实例状态（GuageFile 中标记已完成）|

---

## 6. RS485 vs PLC 通道差异

| 维度 | RS485 (`SchedTask_485`) | PLC (`SchedTask_dot42`) |
|------|------------------------|------------------------|
| 物理层 | `/dev/ttyS*` 串口 | 载波模块 (CCO) → 电力线 |
| 帧封装 | 直接发 DL/T 698.45/645 帧 | 先包 376.2 帧再发串口 |
| 通信模型 | 同步：发→等→收 | 异步：UDP loopback + 队列 |
| 通道线程 | 无独立线程 | `dlt698_42` 线程管理串口 |
| 并发 | 串行 | 可并行（多电表同时抄）|
| 最大电表数 | ~32 (RS485 总线) | ~200+ (载波网络) |

---

## 7. 事件系统 (14 种事件)

`CScheduler` + `CEvent` + `CHandler` + `CSlot` 组成事件驱动核心：

| 事件 | 触发条件 | Handler |
|------|---------|---------|
| SoftDog | 周期 (60s) | 喂硬件看门狗 |
| CheckTaskUpdate | MSG_MTR_TASK_UPDATE 到达 | 读 IPC 更新计数 |
| TransTaskReady | g_transtask_nbr > 0 | 执行透传任务 |
| ImpTaskReady | g_currtask_nbr > 0 | 执行重要任务 |
| BroadTaskReady | 广播任务就绪 | 执行广播任务 |
| SchedTaskReady | 定时触发 | 执行定时抄表 |
| InitMetering | 启动/状态切换 | 重新评估状态 |
| ... | | |

```cpp
// 注册模式
o_scheduler.Attach(&s_softdog);       // 喂狗
o_scheduler.Attach(&s_initMetering);  // 初始化
o_scheduler.Attach(&s_impMetering);   // 重要任务
o_scheduler.Attach(&s_broadMetering); // 广播
o_scheduler.Attach(&s_schedMetering); // 定时
// ... 共 14 个

// 主循环 100ms 轮询
void CScheduler::Update() {
    for (auto& slot : m_slots) {
        if (slot->m_event.BeActive())
            slot->m_handler.Proccess();  // 同步阻塞调用
    }
}
```

---

## 8. 全局状态

```cpp
// 核心全局变量
int g_metering_stauts = INI_METERING;     // 当前状态
int g_transtask_nbr = 0;                  // 待执行透传任务数
int g_currtask_nbr = 0;                   // 待执行重要任务数
uint8_t g_topRunPri = 0;                  // 全局最高优先级 (共享内存)
```

---

## 9. 优先级系统

```
透传任务 (g_transtask_nbr)  >  重要任务 (g_currtask_nbr)
    >  广播任务  >  定时任务 (SchedTask)
```

优先级通过 `g_metering_stauts` 状态机体现：高优先级状态先执行，执行完毕才进入下一优先级状态。

---

## 10. 启动流程

```
main() → StartWork()
    1. 读取配置 (g_cmdopt)
    2. 初始化通道 (InitChannel)
    3. 创建 Task 对象 (SchedTask/RandTask/BroadTask/TransTask)
    4. 初始化事件系统 (Attach 14 个 Handler)
    5. 如果是 PLC 通道: regi_schd(DataToCCO, DataFromCCO) + start_thrd()
    6. 进入主循环: while(1) { scheduler.Update(); }
```

---

## 11. 与 gathermng 对比

| 维度 | gathermng | gatherexe |
|------|-----------|-----------|
| 实例数 | 1 个 | N 个 (每通道 1 个) |
| 职责 | 任务调度/分解 | 任务执行 |
| 对外接口 | 无 | RS485/PLC |
| 主要 IPC | 消息队列 (发送) | 消息队列 (接收) |
| 持久化 | 读参数配置 | 读 GuageFile + 写 DB |
| 运行周期 | 1s | 100ms |

---

## 12. 设计要点

1. **多进程隔离**: 每通道独立进程, 崩溃不影响其他通道或主调进程
2. **非抢占状态机**: 简单可控, 但透传延迟可能达数十分钟
3. **GuageFile 解耦**: gathermng 写入 → gatherexe 读取, 异步、持久、可恢复
4. **事件驱动 + Observer**: 14 种事件通过统一调度器分发, 扩展新事件不改 Scheduler

---

## 13. 设计模式与架构分析

### 13.1 GoF 设计模式

| 模式 | 出现位置 | 类型 |
|------|---------|------|
| **Observer** | Scheduler/Event/Handler/Slot | 行为型 |
| **Strategy** | TaskBase → SchedTask_485/dot42, RandTask_485/dot42 | 行为型 |
| **Template Method** | TaskEntity (Begin→Excute→End), TaskBase (Start→Run→Stop) | 行为型 |
| **State** | g_metering_stauts + Handler 映射 | 行为型 |
| **Facade** | CInstanceOperator → MultiPool → Pool → GuageFile+ExcuteList | 结构型 |
| **Factory** | StartWork() 条件 new 创建 Task 对象 | 创建型 |
| **Proxy** | CInstanceMultiPool 延迟初始化 | 结构型 |
| **Adapter** | CSimpleSlot 统一 Event+Handler 到 Slot | 结构型 |

### 13.2 架构模式

| 模式 | 出现位置 |
|------|---------|
| **Repository** | GuageFile 作为多进程共享实例仓库 |
| **Event-Driven** | Scheduler 事件循环 (100ms 轮询) |
| **Layered** | startup → framework → channels → instanceoperator |
| **Pipeline** | GuageFile→Query→PreSend→Begin→Excute→End→DB |

### 13.3 反模式

1. **全局变量满天飞**: `g_metering_stauts`, `g_currtask_nbr` 等被所有 Handler 读写, 隐式耦合
2. **Handler 承担过多职责**: `CInitMeteringStautsHandler::TaskUpdateMsgProc()` 同时做 5 件事
3. **重复代码**: `CInstancePool` vs `CTransInstancePool` (70%+), SchedTask_485 vs SchedTask_dot42 结构相同
4. **非抢占状态机**: 见 3.2 节
5. **编译期多态替代运行时多态**: `#if defined(TTYPE_*)` 和 `#if defined(LOCALE_*)` 超过 60 个条件编译块

### 13.4 设计模式速查表

| 模式 | 出现位置 | 类型 |
|------|---------|------|
| **Observer** | Scheduler/Event/Handler/Slot | GoF 行为型 |
| **Strategy** | TaskBase → SchedTask_485/dot42, RandTask_485/dot42 | GoF 行为型 |
| **Template Method** | TaskEntity (Begin→Excute→End), TaskBase (Start→Run→Stop) | GoF 行为型 |
| **State** | g_metering_stauts + Handler 映射 | GoF 行为型 (简化版) |
| **Facade** | CInstanceOperator → MultiPool → Pool → GuageFile+ExcuteList | GoF 结构型 |
| **Factory** | StartWork() 的 if-else new | GoF 创建型 (简化版) |
| **Proxy** | CInstanceMultiPool 延迟初始化 | GoF 结构型 |
| **Adapter** | CSimpleSlot 统一 Event+Handler 到 Slot | GoF 结构型 |
| **Repository** | GuageFile 作为多进程共享实例仓库 | 架构模式 |
| **Event-Driven** | Scheduler 事件循环 (100ms 轮询) | 架构模式 |
| **Layered** | startup → framework → channels → instanceoperator | 架构模式 |
| **Pipeline** | GuageFile→Query→PreSend→Begin→Excute→End→DB | 架构模式 |

---

## 14. PLC载波通道完整数据流

### 14.1 通信机制全景

抄表数据流经以下传输媒介：

```
┌─────────────────────────────────────────────────────────────────────┐
│                       通信机制全景                                    │
│                                                                     │
│  gatherexe (SchedTask_dot42)        unify42 (dlt698_42线程)          │
│  ┌──────────────────────┐          ┌──────────────────────────┐     │
│  │ MessageQueue          │          │                          │     │
│  │ MSG_PARALLELTASK+port │←─────────│  RxParallelMsg 分发结果  │     │
│  │ (System V消息队列)     │ 消息队列  │                          │     │
│  │                       │          │                          │     │
│  │ Nodelink UDP:5310     │←─UDP───→│  fd_uplnk (UDP监听)      │     │
│  │ Datalink UDP:5311     │←─UDP───→│  fd_dnlnk→串口→载波模块   │     │
│  │                       │          │                          │     │
│  │ SrvTbl (seq→RxPort)   │ 内存      │  select() 多路复用        │     │
│  │ GatherDataMgr Tx/Rx   │          │  dnlnk_send/recv         │     │
│  └──────────────────────┘          └──────────────────────────┘     │
│                                                                     │
│  传输方式总结:                                                        │
│  ① UDP loopback (127.0.0.1:5310/5311) — 进程间传递抄表帧             │
│  ② 串口 UART (/dev/ttyS*) — 与物理载波模块通信                       │
│  ③ System V 消息队列 (MSG_PARALLELTASK) — 响应结果回传               │
│  ④ 共享内存 (SHM_TERM_DATA) — 任务实例/参数读写                      │
│  ⑤ 函数指针回调 — 跨进程边界的数据交接                               │
└─────────────────────────────────────────────────────────────────────┘
```

#### ① UDP loopback (127.0.0.1)

**gatherexe ↔ unify42(dlt698_42线程)** 之间主要通信方式。

| 链路 | 端口 | 方向 | 数据结构 | 用途 |
|------|------|------|---------|------|
| **Nodelink** | 5310 | gatherexe → unify42 | `ldr_param_t` | 节点上报 + 抄表帧下发 |
| | | unify42 → gatherexe | `ldr_param_t` (回传) | 抄表确认/跳过 |
| **Datalink** | 5311 | unify42 → gatherexe | `str_param_t` | 抄表响应数据回传 |
| | | gatherexe → unify42 | 16字节确认 | 数据已收确认 |
| **Uplnk** | 4050 | gatherexe → unify42 | 控制命令 | 启动/暂停/重启/随机抄表等 |

特点：
- 使用 `SOCK_DGRAM` (UDP)，无连接，低延迟
- 数据包 ≤ 2048 字节
- timeout=200ms，超时即跳过不阻塞
- `select()` 轮询周期：并行模式 100ms，串行模式 1s

#### ② 串口 UART

dlt698_42 线程通过串口直接与物理载波模块 (CCO) 通信：

```cpp
// 打开串口
fd_dnlnk = serial_open(_SERIAL_DNLNK, baud);  // 波特率默认 B9600

// 发送到载波模块
int dnlnk_send(int fd, u_int8_t *buf, u_int32_t len) {
    if (modtype == SOFTROUTER)
        return udpskt_send(UDPSKT_DNLNK, buf, len);  // 软路由模式走UDP
    else
        return serial_send(_SERIAL_DNLNK, buf, len);   // 真实模块走串口
}
```

帧格式：DL/T 376.2-2009 (`0x68` 起始, `0x16` 结束, 含校验和)

#### ③ System V 消息队列

用于 gatherexe 内部跨 Task 对象传递响应数据：

```cpp
// GatherDataMgr::RxParallelMsg() 发送
MessageQueue task(CMessgeKeyMan::MSG_PARALLELTASK + port);
task.Send(&msg, sizeof(msg), 0xf209, IPC_NOWAIT);
```

#### ④ 函数指针回调

跨进程边界核心机制 — unify42 线程通过函数指针调用 gatherexe 函数：

```cpp
// unify42.h 定义的函数指针类型
typedef int (*ldrinst_t)(ldr_param_t *param);  // 取下一帧
typedef int (*strdata_t)(str_param_t *param);  // 存响应数据

// 注册 (startup.cpp:686)
regi_schd(DataToCCO, DataFromCCO);

// dlt698_42.cpp 内部存储为全局函数指针
ldrinst_t ldrinst = NULL;   // → 指向 DataToCCO
strdata_t strdata = NULL;   // → 指向 DataFromCCO
```

**这是整个数据流最关键的设计决策**：unify42 编译为独立模块（独立线程），不直接依赖 gatherexe 任何头文件。通过函数指针注册实现编译期解耦。

---

### 14.2 回调函数清单

| 回调指针 | 类型 | 注册函数 | 实现位置 | 调用时机 | 作用 |
|---------|------|---------|---------|---------|------|
| **ldrinst** | `ldrinst_t` | `regi_schd()` | `DataToCCO()` → `GatherDataMgr.cpp:51` | dlt698_42 每轮需要下一帧时 | 从 Tx 队列取帧，填入 `ldr_param_t`，返回 1=有帧/0=无帧 |
| **strdata** | `strdata_t` | `regi_schd()` | `DataFromCCO()` → `GatherDataMgr.cpp:85` | dlt698_42 收到载波模块响应后 | 将响应数据 push 到 Rx 队列 |
| **syncprocess** | `syncprocess_t` | `regi_sync()` | 各厂商实现 | 时间同步后 | 路由时间同步完成通知 |
| **evupload** | `evfunc_t` | `mdctrl_event_start()` | 事件上报回调 | 事件上传模式收到事件 | 事件数据回传 |
| **bdcfunc** | `bdcfunc_t` | `mdctrl_bdc_start()` | 广播回调 | 广播完成 | 广播结束通知 |
| **rgstapply** | `rgstfunc_t` | `rtset_rgst_start()` | 注册结果回调 | 主动注册完成 | 注册节点信息回传 |
| **rgstapply_zj** | `rgstfunc_zj_t` | `rtset_rgst_zj_start()` | 浙江注册回调 | 采集器搜表完成 | 采集器节点信息回传 |
| **reconfimdata** | `reconfimdata_t` | `rtfwrd_monitor()` | 中继确认回调 | 随机抄表中继确认 | 延迟时间确认数据回传 |

**核心两个回调**（抄表数据流直接相关）：

```cpp
// ① DataToCCO — 被 dlt698_42 线程在每个抄表周期调用
//    从 GatherDataMgr 的 Tx 队列取一帧，填到 ldrparam->buf
int DataToCCO(ldr_param_t *param)
{
    LockGuard<SpinLock<>> lock(GatherDataMgr::Lock);
    if (!GatherDataMgr::InstanceTx().IsEmpty())
    {
        GatherData *item = GatherDataMgr::InstanceTx().Front();
        // 1. 拷贝目的地址
        memcpy_r(param->mac, mac_r, 6);
        // 2. 防重：同一电表不允许同时有两个在途帧
        if (GatherDataMgr::IsMacExist(mac)) return 0;
        // 3. 拷贝帧数据到 param->buf
        param->len = item->GetBuffer().GetSize();
        memcpy(param->buf, item->GetBuffer().GetData(), param->len);
        // 4. 记录 seq→RxPort 映射到 SrvTbl（用于回传路由）
        TxData.RxPort = item->GetRxPort();
        TxData.Seq = param->seq;
        GatherDataMgr::SrvTbl[param->seq] = TxData;
        return 1;  // 有帧
    }
    return 0;
}

// ② DataFromCCO — 被 dlt698_42 线程在收到载波响应后调用
//    将响应数据 push 到 GatherDataMgr 的 Rx 队列
int DataFromCCO(str_param_t *param)
{
    LockGuard<SpinLock<>> lock(GatherDataMgr::Lock);
    std::shared_ptr<GatherData> data = GatherDataMgr::InstanceRx().CreatBack();
    GatherData *item = data.get();
    item->SetAddr(param->mac);
    item->SetSeq(param->seq);
    item->SetBuffer(param->buf, param->len);
    GatherDataMgr::InstanceRx().PushBack(data);
    return 1;
}
```

**数据如何跨线程回到 gatherexe 的业务对象**：

```
DataFromCCO() push 到 Rx 队列
    ↓
gatherexe 主循环 PollingTask() 调用
    ↓
GatherDataMgr::RxParallelMsg()
    ↓ 查 SrvTbl[seq] → 得到 RxPort
    ↓
System V 消息队列 MSG_PARALLELTASK+RxPort
    ↓
对应 SchedTask_dot42 的 PollingTask() 收到
    ↓
fetch_data(str_param_t*) → 解析 → 写库
```

---

### 14.3 dlt698_42 线程总体架构

dlt698_42 是整个 PLC/HPLC 载波通道的**通信中枢线程**，运行在 unify42 模块中。它与 gatherexe 进程在**同一个进程空间**内（作为线程），直接操作串口与物理载波模块 (CCO) 交互。

#### 线程启动

```cpp
// startup.cpp:686-689
regi_schd(DataToCCO, DataFromCCO);  // 先注册回调
int res = start_thrd();             // 再启动线程 → pthread_create(&thrd_id, ..., dlt698_42, ...)
```

#### 主循环结构

```cpp
void *dlt698_42(void *param) {
    do {
        // 1. 异常检测 (抄表/注册/组网超时)
        // 2. 厂商特定维护 (时钟同步等, 如 ZHHWR/JSMESH)
        // 3. 事件处理 (停复电等)

        // 4. select() 多路复用 — 核心
        FD_ZERO(&fdsRD);
        FD_SET(fd_dnlnk, &fdsRD);  // 串口/软路由 ← 载波模块数据
        FD_SET(fd_uplnk, &fdsRD);  // UDP    ← gatherexe 控制命令
        // 并行模式超时 100ms，串行模式 1s
        if (select(FD_SETSIZE, &fdsRD, NULL, NULL, &tvTimeOut) != -1)
        {
            // 5. 下行数据处理 (串口 ← 载波模块)
            if (FD_ISSET(fd_dnlnk, &fdsRD))
            {
                dnlnk_recv_long(m_apsPkt, 0);          // 收原始 376.2 帧
                vector<frmappinfo_t> frmlst;
                FrmStruct::Parser(rcvbuf_dn, rcvlen_dn, frmlst);  // 解析 376.2 帧
                for (uint32_t i = 0; i < frmlst.size(); i++)
                {
                    switch (workmode)                    // 按工作模式分发
                    {
                    case RWM_SCHDMETER:  schdmeter_trans(frmlst[i]);  break;
                    case RWM_RANDMETER:  randmeter_trans(frmlst[i]);  break;
                    case RWM_BROADCAST:  broadcast_trans(frmlst[i]);  break;
                    case RWM_SELFSTUDY:  selfstudy_trans();           break;
                    case RWM_ACTREGIST:  actregist_trans(frmlst[i]);  break;
                    case RWM_EVTUPLOAD:  evtupload_trans();           break;
                    case RWM_IDLE:       idle_trans(frmlst[i]);       break;
                    }
                }
            }

            // 6. 上行数据处理 (UDP ← gatherexe)
            if (FD_ISSET(fd_uplnk, &fdsRD))
            {
                rcvlen_up = udpskt_recv(UDPSKT_UPLNK, rcvbuf_up, BUFSIZ, 0, 0);
                proparse();   // 解析命令类型并执行
            }
        }

        // 7. 定时抄表调度 (select 超时后, 每轮执行)
        if (RWM_SCHDMETER)
        {
            __pararell_sched_handler(&ldrparam, ldrinst);
            //           ↓
            //   ldrinst(ldrparam) → DataToCCO() → 取帧
            //   exeinst(ldrparam) → 组376.2帧 → dnlnk_send() → 串口发出
        }

    } while (1);
}
```

#### 工作模式状态机

dlt698_42 线程内部有独立的**工作模式状态机**（不同于 gatherexe 的任务优先级状态机）：

```
                    ┌──────────────────────────────────────┐
                    │                                      │
                    ▼                                      │
              ┌──────────┐    start     ┌──────────────┐  │
   启动 ───→  │ RWM_IDLE │ ──────────→  │ RWM_SCHDMETER │  │
              └──────────┘              │  (定时抄表)    │──┘
                    │ ↑                 └──────────────┘
                    │ │                        │
                    │ │    ┌───────────────────┼───────────┐
                    │ │    ▼                   ▼           ▼
                    │ │┌──────────┐  ┌──────────┐  ┌──────────┐
                    │ ││RWM_RAND  │  │RWM_BROAD │  │RWM_SELF  │
                    │ ││(随机抄表) │  │(广播抄表) │  │(路由学习) │
                    │ │└──────────┘  └──────────┘  └──────────┘
                    │ │    │              │              │
                    │ │    └──────────────┴──────────────┘
                    │ │                  │
                    │ ▼                  ▼
                    │ ┌──────────┐  ┌──────────┐
                    │ │RWM_ACT   │  │RWM_EVT   │
                    │ │(主动注册) │  │(事件上传) │
                    │ └──────────┘  └──────────┘
                    │      │
                    └──────┘
```

**关键区别**：dlt698_42 的 workmode 控制"与载波模块交互什么"，gatherexe 的 g_metering_stauts 控制"抄表优先级决策"。两者通过 UDP Nodelink/Datalink 协作。

#### 三种抄表模式差异化处理

| 模式 | workmode | 帧来源 | 组帧函数 | 响应处理 |
|------|----------|--------|---------|---------|
| **定时抄表** | RWM_SCHDMETER | `ldrinst()` 回调 → DataToCCO → GatherDataMgr Tx队列 | `frm_parallel_meter_build()` → `FrmStruct::FrmParallelSched()` | `schdmeter_trans()` → `strdata()` → DataFromCCO → Rx队列 |
| **随机抄表** | RWM_RANDMETER | `proparse()` 直接解UDP包 | `FrmStruct::FrmRouterRand()` | `randmeter_trans()` |
| **广播抄表** | RWM_BROADCAST | `proparse()` 直接解UDP包 | `FrmStruct::FrmStartBrd()` | `broadcast_trans()` |

#### proparse() 命令分发

`proparse()` 解析来自 gatherexe 的 UDP 控制命令 (uplnk:4050)，按 AFN+DTN 组合分发：

| 命令 (AFN, DTN) | 动作 |
|-----------------|------|
| ROUTER_CTL + CTL_RESTART | 重启抄表 → `rtctrl_restart_42()` |
| ROUTER_CTL + CTL_PAUSE | 暂停抄表 → `rtctrl_pause_42()` |
| ROUTER_CTL + CTL_RESUME | 恢复抄表 → `rtctrl_resume_42()` + 切换 workmode |
| ROUTER_FWD + FWD_MON_SNODE | 随机抄表帧转发 → `FrmStruxt::FrmRouterRand()` + `dnlnk_send()` |
| PAIR_METERING + PAIR_METER | 并行抄表帧转发 → `FrmStruct::FrmParallelSched()` + `dnlnk_send()` |
| CTRL_COMMAND + CTL_START_BROADCAST | 广播 → `FrmStruxt::FrmStartBrd()` + `dnlnk_send()` |
| INNER_DEBUG | 调试命令 |

---

### 14.4 完整数据流向（按步骤）

以下以一个定时抄表任务从组帧到写库的完整路径：

```
═══════════════════════════════════════════════════════════════════
阶段1: 组帧 (gatherexe进程内)
═══════════════════════════════════════════════════════════════════

[1] SchedTask_dot42::Run()                          (SchedTask_dot42.cpp:830)
    │  dlt698_42线程通过UDP Nodelink上报当前要抄的节点
    │  → m_nodelink_s.RecvFrom() 收到 ldr_param_t (mac=电表地址)
    │
    ▼
[2] sched_task(ldr_param_t*)                        (SchedTask_dot42.cpp:667)
    │  → g_mterDocs.GetDocByAddr(tsa, AppliedDocs)  查电表档案
    │  → sched_task_meter(appliedDoc, param)        单表抄表
    │
    ▼
[3] sched_task_meter(meterdoc_t&, ldr_param_t*)     (SchedTask_dot42.cpp:481)
    │  → 设置 param->proto = D42_698_45 / D42_GB645_07 / D42_BYPASS
    │  → 检查重试次数 ≥ 3 → JUMPOVER
    │  → 检查是否有正在执行的任务实体:
    │     有 → taskentity->PackAppFrame(tmpbuff, len)  继续组上一帧
    │     无 → taskquery.Query(QSCHED, instance, mp)    查询新实例
    │        → new SchedTaskEntity_dot42(instance)
    │        → taskentity->PackAppFrame(tmpbuff, len)
    │
    ▼
[4] SchedTaskEntity_dot42::PackAppFrame()           (SchedTaskEntity_dot42.cpp:87)
    │  → PreSendProcess()                           加载档案、解析splitOAD
    │  → pro_io.Init(protocol)                      初始化协议引擎
    │  → pro_io.FormReadDataFrame(instance, gather, mtritem, bb)
    │
    ▼
[5] CProtocolIO::FormReadDataFrame()                (protocolIO.cpp:30)
    │  → meterctrl.FormReadDataFrame()
    │
    ▼
[6] CMeterCtrl::FormReadDataFrame()                 (meterctl.cpp:60)
    │  → 按协议类型分发:
    │     CProtocol69845::FormReadDataFrame()    (698.45 → HPLC)
    │     CProtocol64507::FormReadDataFrame()    (645-07)
    │
    ▼
[7] CProtocol69845::FormReadDataFrame()             (protocol69845.cpp:166)
    │  → GatherCurrentData() / GatherRecordData()
    │  → CGXDLMSClient 构建 698.45 APDU
    │  → 结果: bb (CGXByteBuffer) — 完整的 698.45 应用层帧
    │
    ▼
[8] 帧数据拷贝到 ldr_param_t                         (SchedTask_dot42.cpp:636)
    param->len = len;
    memcpy(param->buf, tmpbuff, len);
    param->sw = MOREITEM;   // 告诉dlt698_42: 还有更多帧要发送
    
    → m_nodelink_s.SendTo(rcvbuff, &t_clientaddr)   回传给 dlt698_42 线程

    【此时数据: ldr_param_t { mac, proto=0x03, buf=698.45帧, len, sw=MOREITEM }】
    【传输方式: UDP loopback 127.0.0.1:5310 (Nodelink)】

═══════════════════════════════════════════════════════════════════
阶段2: 发送到载波模块 (unify42 / dlt698_42线程内)
═══════════════════════════════════════════════════════════════════

[9] dlt698_42 主循环                                (dlt698_42.cpp:335)
    │  RWM_SCHDMETER 模式下, 每轮 select() 超时后:
    │  __pararell_sched_handler(&ldrparam, ldrinst)
    │
    ▼
[10] __pararell_sched_handler()                     (parallel_unify42.cpp:234)
    │  → ldrparam->seq = next_frame_index()         分配帧序号
    │  → ldrinst(ldrparam)                          回调 → DataToCCO()
    │     ├─ 从 GatherDataMgr::InstanceTx() 队列取帧
    │     ├─ 拷贝帧到 ldrparam->buf
    │     └─ 记录 SrvTbl[seq] = {RxPort, addr, deadline}
    │  → prtinst(ldrparam)                          日志打印
    │  → exeinst(ldrparam)                          组 376.2 帧 + 发送
    │
    ▼
[11] exeinst(ldr_param_t*)                          (dlt698_42.cpp:2082)
    │  → frm_parallel_meter_build(ldrparam, ..., lvcaddr, vender)
    │     ├─ 构建 appreqparallel_t { protocol, usrdata=698.45帧, usrlen }
    │     └─ FrmStruct::FrmParallelSched() 包装成 376.2 帧
    │        AFN=0xF1 (PAIR_METERING), DT=0x01 (PAIR_METER)
    │        帧结构: [0x68][长度L][长度L][控制C][376.2用户数据][CS][0x16]
    │                          ↑
    │                    376.2帧头 (dlt3762_fix_hdr_t)
    │  → dnlnk_send(fd_dnlnk, sndbuf_dn, sndlen_dn)
    │
    ▼
[12] dnlnk_send()                                   (dlt698_42.cpp:462)
    │  → serial_send(_SERIAL_DNLNK, buf, len)        真实模块: 串口发出
    │  → udpskt_send(UDPSKT_DNLNK, buf, len)         软路由: UDP发出
    │
    ▼
  ╔══════════════════════════════════════════════════════╗
  ║  物理载波模块 (CCO)                                    ║
  ║  → 通过电力线载波发送到目标电表 (STA)                    ║
  ║  → 电表响应通过电力线载波返回                             ║
  ╚══════════════════════════════════════════════════════╝

    【传输方式: 串口 UART → 载波模块 → 电力线 → 电表】

═══════════════════════════════════════════════════════════════════
阶段3: 接收响应 (unify42 / dlt698_42线程内)
═══════════════════════════════════════════════════════════════════

[13] dlt698_42 主循环 — select() 检测到 fd_dnlnk 可读  (dlt698_42.cpp:286)
    │  → FD_ISSET(fd_dnlnk, &fdsRD)
    │  → dnlnk_recv_long(m_apsPkt, 0)                从串口收原始 376.2 帧
    │  → FrmStruct::Parser(rcvbuf_dn, rcvlen_dn, frmlst)
    │     解析 376.2 帧 → vector<frmappinfo_t>
    │
    ▼
[14] workmode == RWM_SCHDMETER → schdmeter_trans()   (dlt698_42.cpp:1673)
    │  → 检查 AFN==0x06 (INIT_APPLY), DT==0x02 (APY_APPLY_DATA)
    │  → 从 frmappinfo_t 提取:
    │     strparam.proto  = frminfo.data[2]           协议类型
    │     strparam.phase  = frminfo.usrinfoup.phase   相位
    │     strparam.relay  = frminfo.usrinfoup.relay   中继深度
    │     strparam.ackqos = frminfo.usrinfoup.ack_qos 信号质量
    │     strparam.buf    = &frminfo.data[6]           698.45 响应帧
    │     strparam.len    = frminfo.data[5]            响应长度
    │     strparam.seq    = frminfo.usrinfoup.frmseq   帧序号
    │  → response_42(RSP_CONFIRM, strparam)           发送确认给 CCO
    │  → prtdata(&strparam)                           日志打印
    │  → strdata(&strparam)                           回调 → DataFromCCO()
    │
    ▼
[15] DataFromCCO(str_param_t *)                      (GatherDataMgr.cpp:85)
    │  → 创建 GatherData 对象
    │  → SetAddr(param->mac) | SetSeq(param->seq) | SetBuffer(param->buf, param->len)
    │  → PushBack 到 GatherDataMgr::InstanceRx() 的 Rx 队列

    【此时数据: str_param_t { mac, proto, phase, relay, ackqos, seq, buf=698.45响应, len }】
    【传输方式: 函数指针回调 (直接内存操作, 同进程空间)】

═══════════════════════════════════════════════════════════════════
阶段4: 结果回传与解析 (gatherexe进程内)
═══════════════════════════════════════════════════════════════════

[16] PollingTask() 主循环                             (startup.cpp:858)
    │  → GatherDataMgr::RxParallelMsg()
    │
    ▼
[17] GatherDataMgr::RxParallelMsg()                  (GatherDataMgr.cpp:123)
    │  → InstanceRx().Front()                         从 Rx 队列取数据
    │  → FindRxPort(item->GetSeq(), port)             查 SrvTbl 得到目标端口
    │  → 构造 ParallelRxMsg { buf, buflen }
    │  → MessageQueue(MSG_PARALLELTASK+port).Send()   消息队列发送

    【传输方式: System V 消息队列 MSG_PARALLELTASK+port】

    ▼
[18] SchedTask_dot42 的 PollingTask()                 (消息队列接收)
    │  → MessageQueue(MSG_PARALLELTASK+rxPort).Receive()
    │  → g_isfetchdata = 1
    │  → 下一轮 Run() 中:
    │     m_datalink_s.RecvFrom() → fetch_data((str_param_t*)rcvbuff)
    │
    ▼
[19] SchedTask_dot42::fetch_data(str_param_t *)      (SchedTask_dot42.cpp:715)
    │  → 遍历 m_listTask_excuting (正在执行的任务实体列表)
    │  → (*itertask)->TranRecvFrame(param->buf, param->len)
    │  → 按返回码处理:
    │     res == 0:           成功 → 写节点信息 → 删除 entity
    │     res == NEGATIVE_ERROR: 负确认 → 写节点信息 → 删除 entity
    │     res == OP_SPLIT_NEXT:  分帧未完成, 保留 entity 继续
    │     res == OP_FRZTM_ERROR: 冻结时间错误
    │
    ▼
[20] SchedTaskEntity_dot42::TranRecvFrame()          (SchedTaskEntity_dot42.cpp:186)
    │  → pro_io.ProcessReturnedFrame(bb, app)         解析协议响应
    │     → meterctrl.Unpack_ReturnedFrame()
    │        → CProtocol69845::Unpack_ReturnedFrame()
    │           解析 698.45 APDU → 提取 OAD 数据项
    │  → pro_io.Write2DB(m_instance, m_gather, app)   写入数据库
    │     → itemio.Write2DB()
    │  → 处理 splitOAD 分帧逻辑（多个 OAD 分多帧读取）
    │  → 处理负确认/超时/冻结时间错误
    │
    ▼
  ╔══════════════════════════════════════════════════════╗
  ║  数据持久化完成 — 抄表结果写入 dlmsdb                  ║
  ╚══════════════════════════════════════════════════════╝
```

---

### 14.5 关键设计点

#### 帧序号 (seq) 与回传路由

`seq` 是整个数据流的**关键索引**，确保响应能回到正确的 Task 对象：

```
发送: DataToCCO() → SrvTbl[seq] = {RxPort, addr, deadline}
      ↓
响应: DataFromCCO() → Rx 队列（含 seq）
      ↓
路由: RxParallelMsg() → FindRxPort(seq) → 消息队列端口
      ↓
接收: PollingTask() → fetch_data()
```

没有 seq→RxPort 映射，响应无法确定发给哪个 SchedTask 实例（一个通道可能有多个 Task 对象）。

#### 发送队列与回调解耦

`GatherDataMgr` 的 Tx 队列是**生产者-消费者**模式：

- **生产者**: `PollingTask()` → `TxParallelMsg()` → push 帧到 Tx 队列
- **消费者**: `ldrinst()` 回调 → `DataToCCO()` → 从 Tx 队列取帧

这种设计允许 gatherexe 的业务逻辑（组帧）和 unify42 的通信逻辑（发帧）**异步运行**：gatherexe 可以提前组好多个帧放到 Tx 队列，unify42 按自己的节奏逐个取出。

#### 防重机制

```cpp
// DataToCCO() 中: 同一电表不允许同时有两个在途帧
if (GatherDataMgr::IsMacExist(mac)) return 0;
```

#### 超时清理

```cpp
// RxParallelMsg() 末尾: 60 秒超时的 SrvTbl 映射自动删除
void GatherDataMgr::RemoveTimeOut() {
    for (auto i = SrvTbl.begin(); i != SrvTbl.end();) {
        if (i->second.deadline < curTime)
            i = SrvTbl.erase(i);
        else i++;
    }
}
```

---

## 15. 关键文件索引

| 文件 | 主要内容 |
|------|---------|
| `gatherexe/src/startup/startup.cpp` | 启动入口、通道初始化、回调注册 (`regi_schd`) |
| `gatherexe/src/framework/TaskQuery.cpp` | 实例查询 (`Query(QSCHED, ...)`) |
| `gatherexe/src/channels/dot42/SchedTask_dot42.cpp` | PLC 定时抄表调度：`Run()`, `sched_task()`, `fetch_data()` |
| `gatherexe/src/channels/dot42/SchedTaskEntity_dot42.cpp` | 抄表实体：`PackAppFrame()`, `TranRecvFrame()` |
| `gatherexe/src/framework/GatherDataMgr.cpp` | 数据管理器：`DataToCCO()`, `DataFromCCO()`, `RxParallelMsg()` |
| `meterpro/src/protocolIO.cpp` | 协议 IO：`FormReadDataFrame()`, `ProcessReturnedFrame()` |
| `meterpro/src/meterctl.cpp` | 协议分发：按类型路由到 698.45/645-07 |
| `meterpro/src/protocol69845.cpp` | 698.45 协议实现：组帧/解析 |
| `unify42/dlt698_42.cpp` | 载波通信线程：主循环、`proparse()`, `schdmeter_trans()`, `exeinst()` |
| `unify42/dlt698_42.h` | 376.2 协议定义、枚举、回调类型 |
| `unify42/unify42.h` | 数据结构：`ldr_param_t`, `str_param_t`, 回调 typedef |
| `unify42/parallel_unify42.cpp` | 并行调度：`__pararell_sched_handler()`, `frm_parallel_meter_build()` |
| `unify42/frame/framestuct.cpp` | 376.2 帧构建：`FrmParallelSched()`, `Parser()` |
