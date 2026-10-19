# 模块角色

## 1.基本概述

​	`gathermng` 负责将**抽象抄表任务配置参数（参数 6012）转换为具体抄表实例**，写入持久化队列（`GuageFile`），通过 IPC 消息队列通知 `gatherexe` 进程执行。

## 2.核心链路

```shell
参数DB (6011/6012/6014/6016)#电表档案、任务配置、普通采集方案、事件采集方案
  → 加载 g_meterdoc, g_task, g_gather
  → CTaskSubject 判断到期任务 (最小堆调度)
  → CMeteringTaskHandle 生成 oopinstance_t (含时间窗 + OAD分片)
  → CInstancePool 去重 → GuageFile 持久化写入
  → MessageQueue 通知 gatherexe (含通道/优先级)
```

---

# 文件清单

## 1.头文件 (`include/`)

| 文件 | 说明 |
|:----:|:----:|
| `InstanceKey.h` | 普通/事件实例去重索引。键 = `(meteraddr, gatherid, taskid)`，值 = `list<oopinstance_t>` |
| `InstancePool.h` | 普通/事件实例池。管理 `CGuageFile` 持久化 + `CInstanceKey` 去重 |
| `MeteringTaskHandle.h` | **核心**。任务→实例转换器。 |
| `ParamFileChangeEvent.h` | 监控 7 类参数文件变更，触发日志重配或进程重启 |
| `SchedTask.h` | 包装器：`CTaskEvent`(时间触发器) + `CMeteringTaskHandle`(实例生成) |
| `TaskManage.h` | 顶层协调器。主循环、IPC发送、文件检测、每日维护 |
| `TaskSubject.h` | 观察者模式 + 最小堆(<time_t>)调度 |
| `TransInstanceKey.h` | 透明传输实例去重索引。键 = `insid` |
| `TransInstancePool.h` | 透明传输实例池。使用 `CTransGuageFile` |

## 2.源文件 (`src/`)

| 文件 | 说明 |
|:----:|:----:|
| `main.cpp` | 进程入口。初始化日志、加载参数、启动 `CTaskManage::RunMain()` |
| `InstanceKey.cpp` | 去重索引实现。重复条件: splitoad + exestarttm + exeendtm 全相同 |
| `InstancePool.cpp` | 实例池实现。`InitPool/InsertInstance/LoadInstanceIndex` |
| `MeteringTaskHandle.cpp` | **最复杂文件**。`NormalTaskHandle/EventTaskHandle/TransTaskHandle` + OAD分片 + 时间窗计算 |
| `ParamFileChangeEvent.cpp` | 每5秒检测变更；非日志变更 → `_exit(0)` |
| `SchedTask.cpp` | Update() 薄层转发 |
| `TaskManage.cpp` | RunMain() 主循环 (1Hz) + TaskSchdule() + 每日23:30整理 |
| `TaskSubject.cpp` | Notify() 实现：最小堆pop → 通知所有观察者 → 注册下次触发时间 |
| `TransInstanceKey.cpp` | 透明去重：settime + exestarttm + expiretm 全相同 = 重复 |
| `TransInstancePool.cpp` | 透明实例池实现 |

---

# 全局数据结构

```cpp
vector<oop_MeterBase_t>   g_meterdoc;       // 6011 所有计量点
vector<oop_task_t>        g_task;           // 6012 任务配置
vector<oop_task_t>        g_acstask;        // ACS 交采任务
vector<normalgather_t>    g_gather;         // 6014 普通采集方案
vector<oop_evtgather_t>   g_evnt_gather;    // 6016 事件采集方案
vector<oop_transGather_t> g_trans_gather;   // 透明传输方案

bool   g_bdelmark = false;                  // 每日一次 GuageFile 删除标记
uint8_t g_mtrMode = 0;                      // HPLC模块模式 (0=并行)
uint8_t g_topRunPri = 0xff;                 // 分/秒级普通任务最高运行优先级
```

---

# 核心类详解

## 1.`CTaskEvent` — 时间触发器

### 1.文件

```shell
 include/TaskEvent.h
 pub/TaskEvent.cpp
```

### 2.构造

```c++
  InitByTaskStartTime(task)
   /*
   1. 获取任务起始时间 taskstart(6012.StartTime)
   2. minstart = max(taskstart, now) — 未来任务用任务时间，否则用当前时间
   3. 按 Interval.scale 对齐时间并计算 m_activeTime:
      - SECOND/MINUTE: minstart - (minstart % interval) 对齐到周期边界
      - HOUR: 对齐到整点，回退到能被 interval 整除的小时
      - DAY: 对齐到当天0点 + 配置时分偏置
      - MONTH: 对齐到当月1号 + 配置日时分偏置
   4.m_activeTime += CalcDelaySecs(task.Delay) — 加延迟偏置
   */
```

### 3.触发 `BeAtctive()`

```c++
if now > m_task_expiretime → return false  (任务已过期)
if m_activeTime > now     → return false  (未到触发时间)
else:
  m_last_active_time = m_activeTime //上次激活时间赋值为当前激活时间
  m_activeTime += interval   (推算下次触发时间)
  return true
```

**关键**: `m_activeTime` 始终存储**下次应触发的时间戳**，触发后自动递增。

---

## 2.`CSchedTask` — 单个调度任务

### 1.文件

`include/SchedTask.h`, `src/SchedTask.cpp`

```cpp
class CSchedTask {
    oop_task_t m_task;                // 任务配置副本
    CTaskEvent m_event;               // 任务执行时间触发器
    CMeteringTaskHandle m_handler;    // 触发后的处理函数
};
```

### 2.`Update()`

```c++
if m_event.BeAtctive()://到达任务触发执行时间
    m_handler.SetActiveTm(m_event.GetLastActiveTime(), m_event.GetActiveTime())
    if TRANS == m_task.GatherType:
        m_handler.DoAction(transinstancelist)
    else:
        m_handler.DoAction(instancelist)
```

**`GetTrigTime()`**: 返回 `m_event.GetActiveTime()`，即下次激活时间，供调度优先级队列使用。

---

## 3 `CTaskSubject` — 调度器 

### 1.文件

`include/TaskSubject.h`, `src/TaskSubject.cpp`

```cpp
class CTaskSubject {
    vector<CSchedTask*> _observers;
    priority_queue<time_t, vector<time_t>, tmComp> m_NotifyTime;  // 最小堆
};
```

`tmComp` 使用 `lhs > rhs` → `top()` = **最早时间**。

### 2.`Notify(instancelist, transinstancelist)`

```cpp
1. if m_NotifyTime.empty() → return
2. nexttime = m_NotifyTime.top()
3. if nexttime > now → return  //(没有任何任务到期)
4. m_NotifyTime.pop()
5. while top() <= nexttime: pop()  //(去重 — 同秒多个任务共享触发点)
6. 遍历所有 _observers:
     observer->Update(instancelist, transinstancelist)
     AddNotifyTime(observer->GetTrigTime())  // 注册下次触发
```

### 3.设计要点

`Notify()` 不是单任务触发 — 而是只要最早时间到期，就**通知所有观察者**，让每个观察者自己判断 `BeAtctive()`。同秒内多个任务可并行生成实例。

**`Detach()`**: 从 vector 中移除并 `delete` `CSchedTask` → `CTaskSubject` **拥有**观察者对象所有权。

---

## 4.`CMeteringTaskHandle` — 实例生成器

### 1.文件

`include/MeteringTaskHandle.h`, `src/MeteringTaskHandle.cpp`

### 2.成员

```cpp
oop_task_t m_task;           // 任务配置 (6012)
bool m_acstask;              // 是否 ACS 虚拟任务
time_t m_lstactivetm;        // 上次激活时间 (=区间起点)
time_t m_nextactivetm;       // 下次激活时间 (=区间终点)
```

### 3.三个顶层入口

| 方法 | 触发条件 | 产物 |
|:----:|:-------:|:----:|
| `NormalTaskHandle()` | `GatherType == NORAML(1)` | `map<通道, list<oopinstance_t>>` |
| `EventTaskHandle()` | `GatherType == EVENT(2)` | 同上 |
| `TransTaskHandle()` | `GatherType == TRANS(3)` | `map<通道, list<transinstance_t>>` |

---

### 4.`NormalTaskHandle()` 

##### 1.大致流程

```c++
1. GetNormalGather(task.GatherID, gather) → 加载 6014 采集方案
2. if gather.mstype <= 0 → return false
3. MeterSetSelector mtrsetsel(gather) → 构建计量点筛选器
4. 遍历 g_meterdoc:
   a. mtrsetsel 筛选不匹配 → continue
   b. ACS任务 ↔ ACS计量点 匹配检查
   c. 湖南: 小周期PLC表跳过非关键任务
   d. rtsplitmark = SplitCsd(gather, meter.MeterProtocol)
      (698表=需要CSD拆分, 645表=不需要)
   e. 获取 gatherMtrType (电表=0/水表=1)

   对每个 CSD (gather.csdlist[uj]):

   ┌─ 非698 (645/CJT):
   │  ├─ 水表 (CJT_18804, gatherMtrType==1):
   │  │   遍历 CSD 内每个 OAD → 单独创建实例
   │  │
   │  └─ 电表 (645_07):
   │       ├─ freezeType = csdlist[uj].value[0] (csdtype==1时)
   │       ├─ 跳过 0x20210200 (冻结时间OAD)
   │       │
   │       ├─ 冻结曲线 (0x50020200 && !instant2freeze):
   │       │   ├─ 非块抄: 每个OAD → TranLoadCurveOad 转换 → 逐OAD实例
   │       │   └─ 块抄: 创建实例 splitoad=0x06000001 → SplitInstance 拆分时间
   │       │
   │       └─ 普通/instant2freeze:
   │            遍历OAD → CreateInstanceBasic → 设置 splitoad
   │            gathertype==4 → SplitInstance 拆分时间
   │            黑龙江 gathertype==10 → 特殊 0x4d020200
   │
   └─ 698:
        ├─ nSplitLimite = GetSplitswitch()
        │   (默认4, 从 /mtdpart1/oadsplitswitch 读取, 范围3-4)
        │   ACS表 → nSplitLimite=0xff (不限制)
        │   黑龙江测试模式 → nSplitLimite=4
        │
        ├─ CreateInstanceBasic → csdlstpos=uj
        │
        ├─ gathertype==3 (日冻结):
        │   ├─ 山东/湖北: 特殊时间调整
        │   ├─ 湖北: HubeiSplitInstance
        │   ├─ 3小时任务 → 拆成3个1小时
        │   ├─ 载波块抄 → nSplitLimite=3 (每次最多3个OAD)
        │   └─ SplitOad → 每分片 AddInstance
        │
        ├─ gathertype==4 (重试):
        │   └─ SplitInstance → SplitOad → AddInstance
        │
        └─ 其他:
              ├─ 负荷曲线冻结 (0x50050200): 逐OAD单独实例
              ├─ gathertype==0 (当前值): 跨CSD合并OAD → SplitOad
              ├─ 黑龙江 gathertype==10: 特殊 0x4d020200
              └─ 其他定时采集: SplitOad → AddInstance

   if !rtsplitmark: break  // 698表只处理第一个CSD
```

##### 2.关键分支条件

| 条件组合 | 处理路径 |
|:--------:|:-------:|
| 水表 | 逐OAD实例 |
| 645电表 + f`reezeType`=0x50020200 + !instant2freeze | 冻结曲线(块抄/非块抄) |
| 645电表 + 其他 | 逐OAD实例 |
| 698 + gathertype=3 | 日冻结分片 |
| 698 + gathertype=4 | 重试拆分时间 |
| 698 + freezeType=0x50050200 | 负荷曲线逐OAD |
| 698 + gathertype=0 | 当前值跨CSD合并 |
| 698 + 其他 | 定时采集分片 |

---

### 5.`EventTaskHandle()` 

```c++
1. GetEventGather(task.GatherID, evtgather) → 加载 6016
2. MeterSetSelector 筛选计量点 (跳过ACS)
3. 遍历 evtgather.fileroad[j]:
   - 非698: splitoad = fileroad[j].value[0]
   - 698:   splitoad = fileroad[j].num (OAD数量)
   - AddInstance
```

不做时间拆分和OAD分片。

---

### 6.`TransTaskHandle()`

```
1. GetTransGather(task.GatherID, gather)
2. 遍历 planContentList[i].frameSeqId[j]:
   - nInstanceID = (gatherid << 24) | (schemeid << 8) | frameid
   - FindMeterDocByTsa 查找计量点
   - 从 DBLiteInterface (OI=0x6018) 查询 settime
     查询失败 → continue (跳过该帧)
   - CreateInstanceBasic → AddInstance
```

---

## 5.`CreateInstanceBasic()` — 时间窗计算

填充 `oopinstance_t` 的时间属性：

```c++
//设置 meterseq, tsa, priorit=task.RunPri, taskid, gatherid
borntm = max(now, m_lstactivetm);
prioritygrad = 0;

//按 Interval.scale:

SECOND/MINUTE:
  exestarttm = m_lstactivetm
  guagestartm = m_lstactivetm
  exeendtm = m_nextactivetm - 1
  expiretm = m_nextactivetm - 1
  	+ freezetype==1: exestarttm -= delay
  	+ freezetype==2: exestarttm -= delay, exeendtm += interval_min*60
  	+ 240分钟任务: 调整为 7-19点窗口

HOUR:
  guagestartm = m_lstactivetm + 偏置分钟
  exestarttm = 对齐整点
  exeendtm = exestarttm + interval*3600 - 1
  expiretm = guagestarttm + interval*3600 - 1

DAY:
  guagestartm = m_lstactivetm + 偏置时分
  exestarttm = 当天0点
  exeendtm = 当天0点 + interval*86400 - 1
  expiretm = exeendtm

MONTH:
  guagestartm = m_lstactivetm + 偏置日时分
  exestarttm = 当月1号0点
  exeendtm = 下个interval月 - 1
  expiretm = exeendtm

通用后处理:
  if HOUR/MINUTE/SECOND:
    if totalmin < 61 && freezetype==2:
      expiretm = guagestartm + totalmin*60*2 - 1
    if gathertype==3 (日采集):
      回退一个周期: exestarttm -= totalmin*60

  if expiretm < now: expiretm = exeendtm  (保底)
```

---

## 6.`SplitOad()` — OAD分片算法

**目的**: 698协议响应不超过 ~255 字节，需将OAD列表分组。

```cpp
SplitOad(vtSplitOad, OadArray[], ArrayNum, relatedOadNum, SplitNum):
  nStart = ArrayNum - relatedOadNum
  nCount = 0, RemaidNum = relatedOadNum, i = 0

  while i < SplitNum && RemaidNum > 0:
    oad = OadArray[nStart + nCount]
    if 高精度项 (0x00000400): i += 2
    else if 需量项 (0x10/0x20/0x30/0x40xxxxx): i += 3
    else if 长数据项 (0x00~0x80xxxxxx): i += 2
    else: i += 1

    if i < SplitNum: nCount++, RemaidNum--

  nSplitOad = (nStart << 8) | nCount  // 高16位=起始下标, 低8位=本组OAD数
  vtSplitOad.push_back(nSplitOad)

  if RemaidNum > 0: 递归处理剩余 OAD
```

**分片编码**: `nSplitOad = (起始CSD下标 << 8) | 本组OAD数量`

---

#### 4.4.6 `TranLoadCurveOad()` — 冻结曲线OAD映射

将 698 协议的冻结数据 OAD 映射为 645 协议可识别的负荷曲线 OAD:

```
0x20000200/0x20000201 (电压)     → 0x061001ff / 0x06100101 (单相→0x06100101)
0x20010200/0x20010201 (电流)     → 0x061002ff / 0x06100201
0x20040200/0x20040201 (有功功率)  → 0x061003ff / 0x06100300
0x20050200/0x20050201 (无功功率)  → 0x061004ff / 0x06100400
0x200a0200/0x200a0201 (功率因数)  → 0x061005ff / 0x06100500
0x00100200/0x00100201 (正向有功)  → 0x06100601
0x00200200/0x00200201 (反向有功)  → 0x06100602
0x00300200/0x00300201 (无功1)     → 0x06100603
0x00400200/0x00400201 (无功2)     → 0x06100604
```

---

### 4.5 `CInstanceKey` — 普通/事件实例去重索引

**文件**: `include/InstanceKey.h`, `src/InstanceKey.cpp`

```cpp
typedef map<instancekey_t, list<oopinstance_t>> INSTANCE_INDEX_TABLE;
// instancekey_t = {tsa_t meteraddr, uint8_t gatherid, uint8_t taskid}
```

**`IsValidInstance()`**:

```
1. if instance.expiretm < now → return -1 (已过期)
2. key = {instance.tsa, instance.gatherid, instance.taskid}
3. 在 m_keytable 中查找 → 未找到 → return 0 (无重复，有效)
4. 遍历该key对应列表:
     IsDupInstance(new, old):
       if splitoad相同 && exestarttm相同 && exeendtm相同:
         return -2 (重复)
5. return 0
```

**去重条件**: 同表 + 同方案 + 同任务 + 同OAD分片 + 同时间窗 → 重复。

---

### 4.6 `CTransInstanceKey` — 透明传输去重索引

**文件**: `include/TransInstanceKey.h`, `src/TransInstanceKey.cpp`

```cpp
typedef map<transinstancekey_t, list<transinstance_t>> TRANS_INSTANCE_INDEX_TABLE;
// transinstancekey_t = {uint32_t insid}
```

**去重条件**: `insid` 相同 + `settime` 相同 + `exestarttm` 相同 + `expiretm` 相同 → 重复。

---

### 4.7 `CInstancePool` — 普通/事件实例池

**文件**: `include/InstancePool.h`, `src/InstancePool.cpp`

**`InitPool(channelID)`**:

```
1. 获取 GuageFile 目录路径
2. 打开或创建 GuageFile
3. LoadInstanceIndex():
   a. GetInstanceQuan() → n  ← ⚠️ 无上限
   b. oopinstance_t pinsinfo[n]  ← ⚠️ VLA 栈溢出风险
   c. GetGuageFile(pinsinfo, &n)
   d. DeleteInstanceBatch(pinsinfo, n, g_meterdoc) → 删除无效计量点实例
   e. 遍历有效实例 → m_keylist.InsertNewKey
4. if IsFileNeedDel() && g_bdelmark:
     rm -rf 目录 → ClearStaticData → 重建 GuageFile
```

**`InsertInstance(channellist, exelevel)`**:

```
1. 遍历 channellist:
     if m_keylist.IsValidInstance(d) != 0:
       从 channellist 删除 (去重过滤)
2. m_guageFile.InsertInstanceBatch(channellist) → 批量持久化
3. 建立统计:
   - f6035_t: map<taskid, set<meterseqinfo_t>>
   - mptasksize: map<(meterseq,taskid), count>
4. 每任务:
     GetMpTotalNumOfTask → 获取总表数/应采集表数
     CalcMpTaskSize → 填充每表实例数
     CTaskMetering::Init → 初始化 F6035 统计数据库
```

**`GetMpTotalNumOfTask(taskid)`**:

```
1. 在 g_task 中找 taskid → 获取 GatherType
2. NORAML → g_gather 中找 6014 → MeterSetSelector 统计
   EVENT  → g_evnt_gather 中找 6016 → 仅 645_07/698_45 计入 gathertotalNum
   TRANS  → g_trans_gather 中找方案
```

---

### 4.8 `CTransInstancePool` — 透明传输实例池

与 `CInstancePool` 镜像对称，使用 `CTransGuageFile` 和 `CTransInstanceKey`。

**差异**: 键为 `insid`；`DeleteInstanceBatch` 无需 `g_meterdoc`；插入前先收集到 VLA 再验证批量写入。

---

### 4.9 `CTaskManage` — 主进程协调器

**文件**: `include/TaskManage.h`, `src/TaskManage.cpp`

```cpp
CTaskSubject m_schmtrSets;   // 调度器
int m_dayrec;                 // 上次删除标记日
int m_dayorgrec;              // 上次重组日
```

**`Init()` → `LoadTaskSchdulePlan()`**:

```
遍历 g_task + g_acstask:
  new CSchedTask(task, isACS)
  m_schmtrSets.Attach(pschTirgger)
  m_schmtrSets.AddNotifyTime(pschTirgger->GetTrigTime())
```

**`RunMain()` — 主循环 (1Hz)**:

```cpp
for (;;) {
    // 1. 时钟偏差检测
    if (lsttime - now > 5min || now - lsttime > 1hr) {
        if (回拨) system("rm -rf /mtdpart0/record_*.db");
        fflush(log); sleep(5); exit(1);
    }

    // 2. GetPLCModuleInfo() — 已 #if 0 禁用

    // 3. UpdateDelMarkAndReorgnz()
    //    日期变更 → g_bdelmark=true
    //    每天23:30 → Reorganize GuageFile

    // 4. TaskSchdule() ← 核心

    // 5. FileChanged() — 每5秒检查参数变更

    // 6. feedsoftdog() — 每60秒喂狗

    sleep(1);
}
```

**`TaskSchdule()` — 一次调度**:

```
1. instancelist, transinstancelist = 空 map

2. m_schmtrSets.Notify(instancelist, transinstancelist)
   → 所有到期任务生成实例，按通道分组

3. [山东] if g_powerOn < '2' → g_powerOn = '3'

4. 处理透明传输:
   每通道 → CTransInstancePool::InitPool → InsertInstance
   → MessageQueue.Send(MSG_VAL_TRANS_TASK, channel, IPC_NOWAIT)

5. 处理普通/事件:
   每通道 → CInstancePool::InitPool → InsertInstance
   [黑龙江/English 测试模式 PLC表 → sleep(8)]
   构造 IPC 消息:
     PLC: msg[0]=exelevel, msg[1]=1, msg[2]=0, msg[3..8]=0xAAAAAAAAAAAA(广播MAC)
     其他: msg[0]=exelevel
     AC通道: channel=8 (硬编码)
   → MessageQueue.Send(msg, len, channel, IPC_NOWAIT)
```

**通道编码**:

```
J1G6/J2G6: channel = metercomm >> 16
其他:
  RS485 → channel = metercomm & 0xFF
  AC    → channel = metercomm >> 16
  AC 最终硬编码为 8
```

---

### 4.10 `ParamFileChangeEvent` — 参数变更监控

**文件**: `include/ParamFileChangeEvent.h`, `src/ParamFileChangeEvent.cpp`

**监控 7 类参数**:

| 参数 | 检查间隔 |
|------|---------|
| MeterDocParam (6011) | 30s |
| NormalGatherParam (6014) | 10s |
| TaskParam (6012) | 10s |
| LogConfigParam | 默认 |
| EvtGatherParam (6016) | 10s |
| TransGatherParam | 10s |
| Rs485PortParam | 默认 |

**`Doaction()`** — 每5秒执行一次:

```
遍历 7 个 FileChangeEvent:
  if FileChanged():
    if LogConfigParam:
      → 重新配置日志级别/文件名 (不重启)
    else:
      → _exit(0)  // 硬退出，由守护进程重启
```

---

## 五、IPC 消息格式

**消息队列 Key**: `MSG_MTR_TASK_UPDATE`

### 普通/事件实例

```
普通通道: msg[0] = exelevel (EXCUTE_IMPORTANT=0 / EXCUTE_NORMAL=1), len=1

PLC通道:
  msg[0] = exelevel
  msg[1] = 1
  msg[2] = 0
  msg[3..8] = 0xAAAAAAAAAAAA (广播MAC, 6字节)
  len = 9

通道编码:
  J1G6/J2G6: channel = metercomm >> 16
  其他: RS485 → channel = metercomm & 0xFF, AC → channel = 8
```

### 透明传输实例

```
msg[0] = MSG_VAL_TRANS_TASK
```

---

## 六、关键常量

| 常量 | 值 | 说明 |
|------|---|------|
| `MAXTIMEDIFF` | 3600s | 时钟偏差上限 |
| `MAX_TM_SOFTDOG_FEED_TIME` | 10min(Z3G6/SUN8I) / 60min(其他) | 看门狗超时 |
| `MAX_TM_SOFTDOG_WAITE_TIME` | 60s | 喂狗间隔 |
| `MAX_TM_PARAM_CHECK_TIME` | 5s | 参数变更检查间隔 |
| `REORG_HOUR/MIN` | 23:30 | GuageFile 每日整理时间 |
| `SPLIT_SWITCH_LIMIT` | 4 | OAD 分片上限 (每次最多4个普通OAD) |

---

## 七、运行时配置依赖

| 文件路径 | 作用 | 地域 |
|----------|------|------|
| `/home/app/conf/flags/hunan_continue` | 存在 → PLC表跳过<3小时间隔任务 | 湖南 |
| `/home/app/conf/instant2freeze` | 存在 → 07协议即时转冻结 | 山西 |
| `/mtdpart1/oadsplitswitch` | OAD分片上限 (3~4) | 全局 |
| `/home/app/reboot-status` | 上电状态 '1'/'2'/'3' | 山东 |
| `/home/app/power-time` | 断电时间戳 | 山东 |

---

## 八、已知问题和代码异味

### 安全

1. **VLA 栈溢出** (`InstancePool.cpp:162`, `TransInstancePool.cpp:132`):
   `oopinstance_t pinsinfo[GetInstanceQuan()]` — 数量无上限，恶意 GuageFile → 栈溢出。

2. **命令注入** (`InstancePool.cpp:49`, `TransInstancePool.cpp:48`):
   `sprintf(cmdline, "rm -rf %s", filename); system(cmdline)` — filename 可控时危险。

3. **启动无差别删库** (`main.cpp:118-119`): 每次启动 `system("rm -rf /mtdpart0/record_*.db")`。

### 健壮性

4. **`_exit(0)` 硬退出** (`ParamFileChangeEvent.cpp:69`): 跳过析构，GuageFile 未正常关闭。

5. **`printf` 与 `LOG` 混用**: 非特定 locale 使用 `printf` 打印实例时间。

6. **`assert()` 在生产代码中**: `InstanceKey.cpp:110` — release 编译时静默失效。

7. **`assert(0 == true)` 被注释**: `MeteringTaskHandle.cpp:1307` — 原来应崩溃的地方被禁用。

### 可维护性

8. **60+ 处 `#if defined(LOCALE_*)`**: MeteringTaskHandle.cpp 控制流几乎无法维护。

9. **`#if 0` 死代码**: 5 个文件中有大量废弃代码块。

10. **`using std::*` 在头文件中**: 污染所有包含者的命名空间。

11. **单线程 sleep(1) 精度**: sleep(1) + 执行时间 → 周期略超1秒，长期累积漂移。

### 设计

12. **PLC 广播 MAC 硬编码**: `0xAAAAAAAAAAAA` 对所有 PLC 表广播。

13. **AC 通道硬编码为 8**: `channel = 8`。

14. **各处 locale 时间调整无注释**: 240分钟 → 7-19点窗口等逻辑缺业务解释。



# 整体架构与数据流

## 1.进程在系统中的位置

```
┌─────────────────────────────────────────────────────────┐
│                    参数DB (持久化)                       │
│  6011(电表档案) 6012(任务配置) 6014(普通采集方案)        │
│  6016(事件采集方案) 透明传输方案 ACS采集方案              │
└──────────────────┬──────────────────────────────────────┘
                   │ 读取 (CParam)
                   ▼
┌──────────────────────────────────────────────────────────┐
│                   gathermng (本进程)                      │
│                                                          │
│  任务到期判断 → 实例生成 → 去重 → 持久化入队 → IPC通知    │
└──────┬────────────────────┬──────────────────┬───────────┘
       │ 写入               │ IPC消息队列       │ 共享内存
       ▼                    ▼                   ▼
┌──────────────┐  ┌──────────────────┐  ┌──────────────┐
│  GuageFile   │  │   gatherexe      │  │ g_topRunPri  │
│ (持久化队列) │  │ (抄表执行进程)    │  │ (最高优先级)  │
└──────────────┘  └──────────────────┘  └──────────────┘
```

**总结**: 把"每15分钟抄一次表"这种抽象任务配置, 转换成"现在去抄XX表XX数据项"的具体指令, 交给 `gatherexe` 执行。

---

## 2.进程间通信全图

```
                        ┌──────────┐
                        │ 守护进程  │ ← _exit(0) 触发重启
                        │ (daemon) │
                        └────┬─────┘
                             │ 重启
                             ▼
   ┌──────────────────────────────────────────────────────┐
   │                   gathermng                          │
   │                                                      │
   │  ┌─────────┐   ┌──────────┐   ┌──────────────────┐  │
   │  │ 参数加载 │ → │ 任务调度  │ → │ 实例生成+入队    │  │
   │  │ (CParam) │   │(最小堆)  │   │ (GuageFile)     │  │
   │  └─────────┘   └──────────┘   └───────┬──────────┘  │
   │                                       │              │
   │  ┌──────────────────────────┐         │              │
   │  │ 参数变更监控              │         │              │
   │  │ (每5s检查,变更→_exit(0)) │         │              │
   │  └──────────────────────────┘         │              │
   └───────────────────────────────────────┼──────────────┘
                                           │
                      ┌────────────────────┼────────────────────┐
                      │                    │                    │
                      ▼                    ▼                    ▼
              ┌──────────────┐   ┌──────────────┐   ┌─────────────────┐
              │  GuageFile   │   │ MessageQueue │   │   共享内存       │
              │  (文件系统)   │   │ (System V)   │   │ (System V)      │
              │              │   │              │   │                 │
              │ 持久化实例    │   │ Key:         │   │ g_topRunPri     │
              │ 按通道分文件  │   │ MSG_MTR_TASK │   │ (分/秒级普通    │
              │              │   │ _UPDATE      │   │  任务最高优先级) │
              └──────┬───────┘   └──────┬───────┘   └────────┬────────┘
                     │                  │                    │
                     │ 读取             │ 通知               │ 读取
                     ▼                  ▼                    ▼
              ┌──────────────────────────────────────────────────┐
              │                  gatherexe                        │
              │         (抄表执行进程 — 另一个独立进程)            │
              └──────────────────────────────────────────────────┘
```

---

## 3.通信通道汇总

| 通道 | 方向 | 机制 | 内容 |
|:----:|------|:----:|:----:|
| 参数DB | 读 | CParam (共享参数区) | 6011/6012/6014/6016/日志配置/透明传输方案/485端口参数 |
| GuageFile | 写 | 文件系统 (按通道分目录) | `oopinstance_t` 持久化队列 |
| MessageQueue | 写 | System V IPC, Key=`MSG_MTR_TASK_UPDATE` | 通道号 + 优先级, 触发 `gatherexe` 读 GuageFile |
| 共享内存 | 写 | System V IPC, Key=`SHM_TERM_DATA` | `g_topRunPri` (最高 RunPri 值) |
| 标记文件 | 读 | 文件系统 | 地域特性文件 (hunan_continue 等) |
| 软看门狗 | 写 | `/dev/softdog` | 每60秒喂狗, 超时重启系统 |

**关键**: `gathermng` **只写** GuageFile, `gatherexe` **只读** GuageFile。`gathermng` 不直接调用 `gatherexe`, 全部通过 GuageFile + MessageQueue 解耦。参数DB 可被其他进程 (如 gui 菜单) 并发修改 → gathermng 检测到变更 → `_exit(0)` 硬重启全量重载。

---

## 4 数据流向

### 1.输入

```c++
OOP_METER_PARAM (6011)     → g_meterdoc[]      电表档案 (MeterNo, MeterAddr, MeterProtocol, MeterComm...)
OOP_TASK_PARAM (6012)      → g_task[]          任务配置 (ID, GatherType, GatherID, Interval, RunPri...)
OOP_GATHER_NORMAL_PARAM (6014) → g_gather[]    普通采集方案 (id, csdlist[], gathertype, gtherti...)
OOP_EVENT_GATHER_PARAM (6016)  → g_evnt_gather[] 事件采集方案
OOP_TRANS_GATHER_PARAM     → g_trans_gather[]  透明传输方案
```

### 2.处理: 任务 → 实例转换

```
g_task (何时抄) + g_meterdoc (抄谁) + g_gather (抄什么)
         │
         ▼
  oopinstance_t (一个抄表实例)
  ═══════════════════════════
  meterseq     → 计量点序号
  tsa          → 表通信地址
  taskid       → 任务ID
  gatherid     → 方案ID
  splitoad     → OAD分片编码 (起始CSD偏移<<8 | OAD数)
  priorit      → 优先级 1~4
  prioritygrad → 重要(1)/普通(0)
  borntm       → 实例生成时间
  exestarttm   → 数据时间窗起点
  exeendtm     → 数据时间窗终点
  expiretm     → 过期时间
  guagestartm  → 计量开始时间
  ═══════════════════════════
```

### 3.输出: 入队 + 通知

```
oopinstance_t 列表 (按通道分组: RS485/PLC/AC)
  │
  ├─ 1. CInstancePool::InitPool(channel)
  │      └─ 打开/创建对应通道的 GuageFile
  │      └─ LoadInstanceIndex() 加载已有实例 → 去重索引
  │
  ├─ 2. CInstancePool::InsertInstance(list, exelevel)
  │      ├─ 逐条去重: (tsa+gatherid+taskid+splitoad+时间窗) 全相同 → 跳过
  │      ├─ GuageFile 批量写入磁盘
  │      └─ 更新统计DB (F6035: 任务→计量点→实例数)
  │
  └─ 3. MessageQueue.Send(msg, len, channel, IPC_NOWAIT)
         ├─ 普通通道: msg[0]=exelevel, len=1
         ├─ PLC通道:  msg[0..8] 含广播MAC 0xAAAAAAAAAAAA
         └─ AC通道:   channel 硬编码 8
                │
                ▼
         gatherexe 收到 IPC 消息
         → 按 channel 找到对应 GuageFile
         → 读取实例列表 → 逐条执行抄表
```

---

## 5.通道体系

### 1.通道编码 (发给 gatherexe 的 channel 参数)

| 终端类型 | RS485通道 | AC通道 | PLC通道 |
|:-------:|:---------:|:------:|:-------:|
| J1G6/J2G6 | `metercomm >> 16` | `metercomm >> 16` | `metercomm >> 16` |
| 其他 | `metercomm & 0xFF` | 硬编码 `8` | `metercomm >> 16` |

### 2.GuageFile 目录

每个通道类型独立一个 GuageFile 目录。`CGuageFile::GetGuageFileDiretor(channelType, path)` 获取路径。启动时按需创建/打开。过期或空目录 → 每日 `rm -rf` 清理后重建。

---

## 9.6 生命周期

```
启动
  ├─ [山东] 读上电状态/断电时间/刚启动标记
  ├─ 解析命令行 (-v/-h/-D/-l/-m)
  ├─ 加载日志配置
  ├─ ⚠️ 无条件删库: rm -rf /mtdpart0/record_*.db /tmp/record_*.db
  ├─ LoadTask(): 遍历 OOP_TASK_PARAM
  │   ├─ 停用任务(TaskState==2) → DeleteUnusedTaskIns() 清理 GuageFile
  │   ├─ 有效任务 → push g_task + 计算 g_topRunPri
  │   └─ ACS表存在 → FormAcsTask() 生成虚拟任务 → push g_acstask
  ├─ LoadTaskSchdulePlan(): 注册所有任务到最小堆调度器
  │
  ▼
主循环 (每1秒一轮)
  ├─ 时钟跳变检测: 回拨>5min 或 快进>1hr → 删record DB → exit(1)
  ├─ 日期变更 → g_bdelmark=true
  ├─ 23:30 → GuageFile.Reorganize() 清理过期实例
  ├─ TaskSchdule(): 到期任务 → 生成实例 → GuageFile入队 → IPC通知
  ├─ 参数变更检测 (每5s): 日志配置热重载 / 其他 → _exit(0)
  └─ 喂狗 (每60s)
  
退出
  ├─ _exit(0): 参数变更, 守护进程重启
  ├─ exit(1): 时钟异常, 守护进程重启
  └─ return -1: 不可达
```

---

## 9.7 与 gatherexe 的解耦设计

```
gathermng                          gatherexe
─────────                          ─────────
写入 GuageFile ────文件系统────→ 读取 GuageFile
发送 MessageQueue ──System V IPC─→ 接收 MessageQueue (触发读取)
写入 g_topRunPri ──共享内存─────→ 读取 g_topRunPri (调整调度策略)
```

- 两进程可独立启停, 互不阻塞
- GuageFile 作持久化缓冲: gatherexe 暂时不可用时, 实例不丢失
- MessageQueue 仅作通知信号 (非阻塞 `IPC_NOWAIT`), 不承载实例数据
- 参数变更时 gathermng `_exit(0)` 重启, gatherexe 不受影响继续执行当前实例
---

#  完整实例

## 1.场景设定

```c++
698 协议三相电表, 每15分钟抄一次日冻结数据 (正向有功+反向有功+A相电压),
RS485端口1, 任务优先级2(重要)。

当前时间: 2026-06-03 10:15:02
```

## 2.输入数据

```
┌─ 6012 任务配置 (g_task[3])
│   ID=3, GatherType=NORAML, GatherID=8
│   Interval: scale=MINUTE(1), value=15
│   StartTime=2024-01-01 00:00:00
│   RunPri=2, Delay=0, TaskState=1
│
├─ 6014 采集方案 (g_gather, id=8)
│   mstype=0(全选), gathertype=3(日冻结)
│   gtherti: scale=MINUTE, value=15
│   csdlist[0]: type=1(冻结类), num=4
│     value[0]=0x00010200 ← freezeType OAD (跳过)
│     value[1]=0x00100200 ← 正向有功 (要抄)
│     value[2]=0x00200200 ← 反向有功 (要抄)
│     value[3]=0x20000200 ← A相电压  (要抄)
│
└─ 6011 电表档案 (g_meterdoc[50])
    MeterNo=50, MeterAddr=0x39383534333231 (TSA)
    MeterProtocol=OOP_698_45
    MeterComm=0x00010001 (RS485, 端口1)
    MeterLinkmode=Link_THREE (三相)
```

## 3. 时间触发

```
CTaskEvent::BeAtctive() 内部:
  m_activeTime = 2026-06-03 10:15:00  (从 10:00 累加15分钟)
  now = 10:15:02
  m_activeTime <= now → return true
  m_last_active_time = 10:15:00
  m_activeTime += 15min → 10:30:00 (注册下次触发)

CSchedTask::Update():
  m_handler.SetActiveTm(lst=10:00:00, next=10:15:00)
  m_handler.DoAction(instancelist)
```

## 3.实例生成 

```
GetNormalGather(8, gather) → 成功, gathertype=3
MeterSetSelector(gather)(meter[50]) → 匹配
rtsplitmark = SplitCsd(gather, 698) → false (698单CSD)

进入 698 分支 (gathertype=3 日冻结):
  freezetype = (0x00010200 >> 16) = 0x01 (日冻结)
  nSplitLimite = GetSplitswitch() → 4 (默认)

  CreateInstanceBasic(meter[50], oopinstance, 3)
```

## 4.时间窗计算

```
输入:
  m_lstactivetm = 10:00:00
  m_nextactivetm = 10:15:00
  Interval.scale = MINUTE, freezetype = 1

MINUTE 分支:
  exestarttm  = 10:00:00
  guagestartm = 10:00:00
  exeendtm    = 10:14:59
  expiretm    = 10:14:59

  freezetype=1:
    exestarttm -= delay(0) → 不变

后处理 (gathertype=3 日冻结, totalmin=15):
  回退一个周期:
  exestarttm  = 10:00:00 - 15min = 09:45:00
  exeendtm    = 09:45:00 + 15min - 1 = 09:59:59
  expiretm = exeendtm = 09:59:59

固定字段:
  meterseq=50, tsa=0x39383534333231, priorit=2
  schemetype=1, taskid=3, gatherid=8
  borntm=10:15:02, prioritygrad=0
  freezetype=1, csdlstpos=0
```

## 5.OAD 分片

```
输入:
  OadArray = {freezeType, 正向有功, 反向有功, A相电压}
  relatedOadNum = 3 (跳过 freezeType OAD), SplitNum = 4

OAD[1]=0x00100200 (正向有功):
  bLongItem? → Yes (0x0010在长数据集合) → 权重2
  i=0, 0+2=2 < 4 → 收入本组, nCount=1, RemaidNum=2

OAD[2]=0x00200200 (反向有功):
  bLongItem? → Yes → 权重2
  i=2, 2+2=4 >= 4 → 本组停止

→ 分片1: nSplitOad = (1<<8)|1 = 0x0101

递归: OAD[2] → 分片2: 0x0201
递归: OAD[3] (权重1) → 分片3: 0x0301

vtSplitOad = {0x0101, 0x0201, 0x0301}  (3个分片)
```

## 6. 实例入队

```
╔════════════════╦═════════════╦═════════════╦═════════════╗
║  字段           ║ 实例1       ║ 实例2       ║ 实例3       ║
╠════════════════╬═════════════╬═════════════╬═════════════╣
║  meterseq      ║ 50          ║ 50          ║ 50          ║
║  tsa           ║ 0x3938..31  ║ 同          ║ 同          ║
║  taskid        ║ 3           ║ 3           ║ 3           ║
║  gatherid      ║ 8           ║ 8           ║ 8           ║
║  splitoad      ║ 0x0101      ║ 0x0201      ║ 0x0301      ║
║  priorit       ║ 2           ║ 2           ║ 2           ║
║  prioritygrad  ║ 0(IMPORT)   ║ 0           ║ 0           ║
║  freezetype    ║ 1(日冻结)   ║ 1           ║ 1           ║
║  borntm        ║ 10:15:02    ║ 同          ║ 同          ║
║  exestarttm    ║ 09:45:00    ║ 同          ║ 同          ║
║  exeendtm      ║ 09:59:59    ║ 同          ║ 同          ║
║  expiretm      ║ 09:59:59    ║ 同          ║ 同          ║
╚════════════════╩═════════════╩═════════════╩═════════════╝

仅 splitoad 不同 — gatherexe 据此知道抄哪些 OAD
```

## 7.持久化 

```
TaskSchdule() 遍历:
  channelKey = 0x00010001 (RS485, 端口1)

CInstancePool::InitPool(0x00010001):
  → 打开 RS485 GuageFile → LoadInstanceIndex() 加载已有实例到内存索引

InsertInstance({I1,I2,I3}, exelevel):
  逐条 IsValidInstance():
    key={tsa_50, gatherid=8, taskid=3} → 索引无 → 全部有效
  GuageFile.InsertInstanceBatch(3条) → 磁盘写入成功
  更新统计: f6035_t[3]={meter 50}, mptasksize[{50,3}]=3
  exelevel = EXCUTE_IMPORTANT
```

## 8.IPC 通知

```
非PLC, RS485通道:
  channel = 0x00010001 & 0xFF = 1

MessageQueue(MSG_MTR_TASK_UPDATE):
  msg[0]=1(IMPORTANT), len=1
  Send(msg, 1, channel=1, IPC_NOWAIT)
```

## 9.gatherexe 执行

```
gatherexe:
  收到 MessageQueue(ch=1) → 打开 RS485 GuageFile
  → 读取 {I1,I2,I3} + 旧实例 → 逐条组帧:

    I1(splitoad=0x0101): CSD下标1,1个OAD → 0x00100200(正向有功)
      组698帧 → RS485发送 → 表 3938..31 → 响应 → 结果入库

    I2(splitoad=0x0201): CSD下标2,1个OAD → 0x00200200(反向有功)
      组帧 → 发送 → 收 → 入库

    I3(splitoad=0x0301): CSD下标3,1个OAD → 0x20000200(A相电压)
      组帧 → 发送 → 收 → 入库

  → 通道完成, 等待下一条 IPC 消息
```

# 端到端数据流总览

```
          参数DB (静态)                        时钟 10:15:02
               │                                    │
               ▼                                    ▼
  ┌────────────────────────────────────────────────────────┐
  │                    gathermng                            │
  │                                                        │
  │  6012(每15分钟) + 6014(日冻结OAD) + 6011(RS485电表)     │
  │         │                                              │
  │         ▼                                              │
  │  最小堆到期 → CreateInstanceBasic → SplitOad (3分片)    │
  │         │                                              │
  │         ▼                                              │
  │  instancelist[0x00010001] = {I1(0x0101), I2(0x0201),   │
  │                               I3(0x0301)}              │
  │         │                                              │
  │         ▼                                              │
  │  去重 → GuageFile 写入 → MessageQueue.Send(ch=1)       │
  └────────────┬───────────────────┬───────────────────────┘
               │                   │
               ▼                   ▼
        ┌──────────────┐   ┌──────────────┐
        │  GuageFile   │   │ MessageQueue │
        │  (磁盘持久化) │   │ (System V)   │
        └──────┬───────┘   └──────┬───────┘
               │                  │
               ▼                  ▼ (触发)
  ┌────────────────────────────────────────────────────────┐
  │                    gatherexe                             │
  │                                                        │
  │  读 GuageFile → 逐条: splitoad→取OAD→组帧→RS485→电表    │
  │  → 响应解析 → 结果入库                                  │
  └────────────────────────────────────────────────────────┘
```

**关键要点**:

1. **完全解耦**: gathermng 和 gatherexe 无直接调用, 通过文件 + IPC 两个通道松耦合
2. **splitoad 编码**: `(CSD偏移<<8)|OAD数量` → gatherexe 据此从方案中取 OAD 组帧, 无需传完整 OAD 列表
3. **时间窗**: exestarttm/exeendtm 告诉 gatherexe 数据时间范围。日冻结回退一个周期确保数据已冻结
4. **去重**: key=(tsa,gatherid,taskid) + splitoad + 时间窗 → 防重复。重启后从 GuageFile 重建
5. **非阻塞**: IPC_NOWAIT, gathermng 不等 gatherexe。实例已持久化, 不丢失
6. **1任务→多实例**: 1任务 x 3分片 = 3实例。10个电表 → 30实例, 按 RS485/PLC/AC 通道分组入队

---

## 10. 补抄机制

gathermng 没有独立的"补抄模块"。补抄通过 **持久化 + 去重 + 时间窗** 三个机制耦合实现，无需单独的失败检测和重调度。

### 10.1 核心原理：持久化即补抄

```
时间线:
  00:00        00:15        00:30
   │            │            │
   ▼            ▼            ▼
[任务触发]   [任务触发]   [任务触发]
   │            │            │
   ▼            ▼            ▼
生成实例      生成实例      生成实例
窗口[23:45,   窗口[00:00,   窗口[00:15,
  00:00]        00:15]        00:30]
   │            │            │
   ▼            ▼            ▼
┌──────────────────────────────────────────┐
│              GuageFile (磁盘)              │
│                                          │
│  [23:45-00:00] ✅成功 → gatherexe执行完成  │
│  [00:00-00:15] ❌失败 → 留在GuageFile中    │
│  [00:15-00:30] ⏳新生成                    │
└──────────────────────────────────────────┘
                     │
gatherexe 持续执行非过期实例:
  → [00:00-00:15] 的失败实例会被反复重试
  → 直到 expiretm 过期后才被清理
```

**关键**: 失败实例不需要 gathermng 显式"重新生成"——它本来就没被删除，gatherexe 每轮状态机循环都会尝试执行所有非过期实例。

### 10.2 去重机制：防止重复生成

每次任务触发时，gathermng 会为新时间窗生成实例。但需要确保不生成与已有实例重复的实例。

**`CInstanceKey::IsValidInstance()`** (`InstanceKey.cpp:51`):

```
检查链:
  1. expiretm < now? → 已过期 → 拒绝
  2. 查 m_keytable[(gatherid, taskid, tsa)]
  3. 遍历已有实例，比对:
     - splitoad 相同?
     - exestarttm 相同?
     - exeendtm 相同?
     → 全部相同 = 重复 → 拒绝
     → 任一不同 = 新实例 → 接受
```

```cpp
int CInstanceKey::IsValidInstance(oopinstance_t &instance)
{
    // 1. 过期检查
    if (instance.expiretm < time(NULL)) return -1;

    // 2. 查索引: key = (gatherid, taskid, meteraddr)
    instancekey_t key;
    key.gatherid = instance.gatherid;
    key.meteraddr = instance.tsa;
    key.taskid = instance.taskid;
    auto iter = m_keytable.find(key);
    if (iter == m_keytable.end()) return 0;  // 无同key实例 → 接受

    // 3. 逐条比对已有实例
    for (auto& existing : iter->second) {
        if (IsDupInstance(instance, existing) == -2)
            return -1;  // 重复 → 拒绝
    }
    return 0;  // 接受
}

int CInstanceKey::IsDupInstance(const oopinstance_t& o1, oopinstance_t& o2)
{
    // splitoad 不同 → 不重复（不同数据项）
    if (o1.splitoad != o2.splitoad) return 0;

    // 时间窗不同 → 不重复（不同时间点）
    if (o1.exestarttm != o2.exestarttm || o1.exeendtm != o2.exeendtm)
        return 0;

    return -2;  // splitoad + 时间窗都相同 → 重复
}
```

### 10.3 重启恢复：从 GuageFile 重建索引

进程重启后，内存中的 `m_keytable` 丢失。通过 `LoadInstanceIndex()` 从 GuageFile 重建：

**`CInstancePool::LoadInstanceIndex()`** (`InstancePool.cpp:157`):

```
启动流程:
  1. InitPool() → LoadInstanceIndex()
  2. 读 GuageFile 全部实例
  3. DeleteInstanceBatch() — 删除已过期 + 对应电表已归档的实例
  4. 逐个 InsertNewKey() — 重建 m_keytable 索引
  5. 跳过 expiretm < now 的实例
```

```cpp
bool CInstancePool::LoadInstanceIndex(void)
{
    unsigned int quan = m_guageFile.GetInstanceQuan();
    oopinstance_t pinsinfo[quan];
    m_guageFile.GetGuageFile(pinsinfo, &quan);

    // 删除过期 + 已归档电表的实例
    m_guageFile.DeleteInstanceBatch(pinsinfo, quan, g_meterdoc);

    // 重建索引
    for (unsigned int ui = 0; ui < quan; ui++) {
        if (pinsinfo[ui].expiretm < time(NULL)) // 过期跳过
            continue;
        key.gatherid = pinsinfo[ui].gatherid;
        key.meteraddr = pinsinfo[ui].tsa;
        key.taskid = pinsinfo[ui].taskid;
        m_keylist.InsertNewKey(key, pinsinfo[ui]);
    }
}
```

### 10.4 曲线数据补抄：FormRtryMtrInstance

当 `gather.gathertype == 4`（曲线数据）时，不走普通 SplitInstance，而是走 `FormRtryMtrInstance`。

**`CMeteringTaskHandle::FormRtryMtrInstance()`** (`MeteringTaskHandle.cpp:1115`):

```
原理: 曲线数据需要连续N个数据点。如果中间有失败，需要回溯补抄。

gather.gathervalue = N  →  生成 N 个实例，时间回退:

  instance[0]: exestarttm=当前, exeendtm=当前
  instance[1]: exestarttm=当前-div, exeendtm=当前-div
  instance[2]: exestarttm=当前-2*div, exeendtm=当前-2*div
  ...

其中 div = gather.gtherti（采集间隔）:
  - scale=1 → div = value 分钟
  - scale=2 → div = value 小时
  - scale=3 → div = value 天
```

```cpp
int CMeteringTaskHandle::FormRtryMtrInstance(...)
{
    if (4 != gather.gathertype) {
        instancelist.push_back(instance);  // 非曲线 → 直接返回
        return 0;
    }

    // compute div from gather.gtherti
    // gather.gathervalue = 需要补抄的数据点数

    if (3 == gather.gtherti.scale)  // 天
    {
        div = gather.gtherti.value * SECONDS_DAY;
        for (int i = 0; i < gather.gathervalue; i++)
        {
            oopinstance_t temp = instance;
            temp.exeendtm = instance.exeendtm - i * div;    // 逐天回退
            temp.exestarttm = instance.exestarttm - i * div;
            instancelist.push_back(temp);
        }
    }
    // scale=1(分钟) 或 scale=2(小时) 时更复杂:
    //   外层: gathervalue 天 (div = task interval)
    //   内层: 每天 div/rediv 个数据点 (rediv = gather.gtherti)
}
```

### 10.5 补抄触发时机

```
任务触发 (CTaskEvent::BeAtctive)
    │  m_activeTime <= now → 触发
    │  m_activeTime += interval → 预约下次
    │
    ▼
CMeteringTaskHandle::NormalTaskHandle()
    │  遍历所有电表 × 所有 CSD 分片
    │
    ├─ gather.gathertype == 4 (曲线)
    │   └─ SplitInstance() → FormRtryMtrInstance()
    │       → 生成 gathervalue 个回溯实例
    │
    └─ gather.gathertype != 4 (实时/日冻结)
        └─ SplitInstance()
            → 将时间窗拆分为子窗口实例
            → 每个子窗口一个实例
    │
    ▼
CInstancePool::InsertInstance()
    │
    ├─ IsValidInstance() → 去重 (时间窗+splitoad)
    │   重复 → 删除 (已有实例在 GuageFile 中)
    │   非重复 → 保留
    │
    └─ InsertInstanceBatch() → 写入 GuageFile
    │
    ▼
MSG_MTR_TASK_UPDATE → 通知 gatherexe
```

### 10.6 过期清理

| 场景 | 清理方式 |
|------|---------|
| 实例 expiretm < now | `IsValidInstance()` 返回 -1 → 新实例不生成 |
| GuageFile 中过期实例 | `LoadInstanceIndex()` 跳过 + `DeleteInstanceBatch()` 删除 |
| 电表已归档 | `DeleteInstanceBatch()` 按 `g_meterdoc` 过滤删除 |
| GuageFile 全部过期 | `IsFileNeedDel()` → `rm -rf` 整个文件 |

### 10.7 设计评价

**优点**:
- 简单: 无独立重试调度器，复用任务调度周期
- 可靠: GuageFile 磁盘持久化，重启不丢
- 自动: 失败实例自然留在队列中，无需显式状态跟踪

**缺点**:
- 重试间隔 = 状态机轮转周期（可能数分钟），无法区分紧急补抄和普通补抄
- 无最大重试次数限制（仅靠 expiretm 兜底）
- 曲线补抄一次性生成所有回溯实例，可能瞬间产生大量实例（如日冻结补 30 天 × 96 点 = 2880 个实例）
- 失败原因不区分（通信失败 vs 电表拒答 vs 数据无效），全部同等重试
