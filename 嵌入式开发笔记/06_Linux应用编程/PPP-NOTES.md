# PPP 是什么

PPP 是**点对点链路层协议**，运行在一条"字节流式"的点对点物理通道上（串口、Modem 拨号、ISDN、光纤直连、蜂窝拨号等），负责：

1. 把字节流**封装成有边界的帧**，承载网络层协议（主要是 IP）
2. **建立、配置、维护、终结**链路
3. 在链路上动态协商网络层参数（IP 地址、DNS 等）

一句话：**它让"一根会传字节的线"变成"一条能跑 IP 的网络链路"**。

### 协议族定位
```
应用层（HTTP/业务）
传输层（TCP/UDP）
网络层（IP）          ← IP 包被 PPP 封装
数据链路层：PPP       ← 本文主题
物理层（串口/Modem/无线基带）
```

对应 OSI 第二层；同类链路层还有 HDLC、以太网。PPP 不关心物理介质是什么，只要介质能传字节。

---

## 2. 为什么需要 PPP（解决什么问题）

裸串口只是一根字节管道，直接跑 IP 会遇到一堆问题，PPP 逐条解决：

| 问题 | PPP 的解法 |
|------|-----------|
| 字节流没有"包边界"，接收方不知道一段数据到哪结束 | 帧格式：标志字节 `0x7E` 定界 + 长度/校验（RFC 1662） |
| 双方参数（如 MRU、要不要认证、用啥认证）没商量过 | LCP 协商 |
| 没有认证，谁都能连 | PAP / CHAP / EAP 可选认证 |
| 网络层参数（本端 IP、对端网关 IP、DNS）需要动态分配 | NCP（常见 IPCP）协商 |
| 物理线可能异常断开而不自知 | LCP 保活（Echo-Request）与终止流程 |
| 链路上要同时跑多种网络层协议 | 协议字段多路复用 |

不是 PPP 能传 IP 而已——早期点对点链路（SLIP 等）也能传，但没有协商/认证/配置能力。PPP 的增值在**"建链会谈"**。

---

## 3. 帧格式（RFC 1662，类 HDLC）

```
7E | FF | 03 | 协议(2B) | 信息(变长) | FCS(2B) | 7E
标志 地址 控制  协议号      载荷         校验     标志
```

- **标志 `0x7E`**：帧边界。内容里若出现 `0x7E` 需**字节填充转义**（`0x7D` + 异或 0x20），保证标志唯一
- **地址 `0xFF` / 控制 `0x03`**：固定值（PPP 简化 HDLC，不寻址）
- **协议号**（关键字段，多路复用依据）：
  - `0x0021` IPv4
  - `0xc021` LCP（链路控制）
  - `0xc023` PAP 认证 / `0xc223` CHAP 认证
  - `0x8021` IPCP（IPv4 网络控制）
  - `0x8057`/`0x8107` 压缩类，`0x003d` 多链路等
- **信息**：网络层包或协商报文
- **FCS**：帧校验（错误帧丢弃）

`asyncmap`/`escape` 等配置控制哪些控制字符要做转义（对应 `0x7D` 填充规则）。

---

## 4. 四个子协议（PPP 分阶段干活）

PPP 一条链路的生命周期由三种子协议驱动，外加数据阶段：

### LCP — 链路控制协议（0xc021）
- 建链时先跑：协商 MRU、认证协议、asyncmap、魔数（环回检测）等
- 维护：链路质量报告、Echo-Request/Reply 保活（探测对端是否还活着）
- 拆链：Terminate-Request 终止链路

### 认证协议（可选）
- **PAP**（0xc023）：明文传用户名/密码，简单不安全
- **CHAP**（0xc223）：挑战-应答，口令不明文上线
- **EAP**：可扩展（证书等）
- 认证与否、用哪种，在 LCP 里先协商好；认证失败则链路终止

### NCP — 网络控制协议（一组）
每个网络层协议配一个 NCP。跑 IPv4 用 **IPCP**（0x8021），协商：
- 本端 IP 地址（可自动分配）
- 对端（通常是运营商/对端路由器）IP
- DNS 服务器（`usepeerdns` 场景关键）

NCP 完成后进入**数据阶段**，才能真传 IP 包。

### 数据阶段
协议字段直接写网络层协议号（如 `0x0021`），载荷即 IP 包。

---

## 5. 建链状态机（RFC 1661 状态）

```
Dead(物理不通)
  ↓ 物理层 up
Establishing      ← 跑 LCP，协商参数
  ↓ LCP 完成
Authenticating    ← 认证（可选）
  ↓ 认证通过 / 无需
Network           ← 跑 NCP/IPCP，拿 IP
  ↓ IPCP 完成
Open              ← 数据阶段，传 IP
  ↓ 断开/保活失败
Terminating → Dead
```

排障时对照此表看卡在哪一阶段，定位最快。

---

## 6. Linux 上 PPP 的实现分工

Linux 把 PPP 拆成三块：

| 部分 | 是谁 | 干什么 |
|------|------|--------|
| **数据面** | 内核 `ppp_generic` + `ppp_async`（/dev/ppp 及 `ppp0` 网卡） | 每包 HDLC 组帧/解帧、协议分发；应用 socket 走 `ppp0` 网卡 |
| **控制面** | `pppd` 守护进程（用户态） | 跑 LCP/IPCP/认证状态机、收发协商帧、管配置与生命周期 |
| **建立物理链路** | `chat` 或自定义 connect 程序 | 在进 PPP 前，向 Modem 发 AT 指令建承载、把线弄进"数据态" |

关键认知：
- **pppd 不逐包搬数据**。它建完链路后退居控制面（保活/重协商）。
- 协商控制帧（LCP/IPCP）本质也是 PPP 帧，同样过内核封装，但**应答逻辑**在 pppd。
- 数据面只认"帧怎么封装、往哪送"，不决定"怎么谈"。

### connect / disconnect 脚本机制
pppd 不自己拨号，它把建链动作外包给一个可执行程序：

- `connect "..."`：pppd **fork 该程序**（以 `/bin/sh -c` 运行），并把**自己已打开的串口设备作为子进程的 stdin/stdout**；子进程负责发 AT、拨号，成功后退出码为 0，pppd 随即在同一设备上开始 PPP 协商
- `disconnect "..."`：断开时跑，发 AT 挂断、把模块退回命令态

经典实现是 `chat`（写 expect/send 脚本）；任何能完成同样任务的程序都行——很多厂家用自家拨号程序替换 `chat`，行为等价。

---

## 7. 蜂窝（GPRS/3G/4G）上的 PPP

### 建立
- 主机向模块发 AT 数据呼叫指令：`ATD*99***1#`（GSM/UMTS 的 PPP 型 PDP 数据呼叫；CDMA 常用 `ATDT#777`）
- 模块注册网络、建立承载后进入**数据模式**（PPP 透传）
- 主机接着跑 pppd：LCP/IPCP 协商，拿到 IP → 得到 `ppp0` 这类接口 + 默认路由

### 数据去哪了（重要概念）
PPP 帧的终点**不是远端服务器，而是运营商网关 GGSN/PGW**：
```
主机 ←串口PPP帧→ 模块 ←空口无线帧(PDCP/RLC/MAC，PPP 只是载荷)→ 运营商 GGSN/PGW
  PPP 在 GGSN 解封装、还原 IP，从这里接入骨干网
  → 骨干网/公网 → 远端服务器（服务器只见普通 IP 包，永远看不到 PPP 帧）
```

### 模块的两种角色（对照）
1. **PPP 透传模式**：模块当哑通道，PPP 全在主机侧做——主机拿 `ppp0` + 自己的 IP 栈
2. **内置协议栈 / AT socket**：模块内部自己完成 TCP/IP 封装，主机不用 PPP，串口上运的是裸应用数据（模块按你给的远端地址自己建 socket）

选哪种是架构取舍：PPP 换主机侧标准化（普通 socket、不绑模块）；AT socket 省主机资源但绑死模块且传输能力受串口/单包限制。

---

## 8. 常见配置项速查

| 配置 | 含义 |
|------|------|
| `name <user>` | 本端认证用户名 |
| `noauth` | 不要求对端认证 |
| `<设备>` | 串口设备，如 `/dev/ttyS0`、`/dev/mux0` |
| 波特率数字 | 串口速率，如 `38400` |
| `connect "..."` | 建链脚本（替代/包装 chat） |
| `disconnect "..."` | 断链脚本 |
| `defaultroute` | 成功后把 ppp 设为默认路由 |
| `usepeerdns` | 采用对端协商给的 DNS |
| `asyncmap <hex>` | 控制字符转义位图 |
| `escape xx` | 强制转义某字符 |
| `mtu` / `mru` | 收发最大帧 |
| `persist` | 断线后自动重拨 |
| `crtscts` | 硬件流控 |
| `unit n` | 指定网口编号（pppn） |
| `pap-secrets` | PAP/CHAP 口令文件路径 |

---

## 9. 排障速查

| 现象 | 查什么 |
|------|--------|
| 拨号后没有 `ppp0` | connect 是否退出码 0；AT 是否到 CONNECT |
| 有 `ppp0` 但无 IP / 不是 up | IPCP 是否完成；是否 `noauth`/口令不对 |
| 一直卡 LCP | 对端不回 LCP；asyncmap/转义不匹配；串口速率 |
| 认证失败 | 用户名/口令、CHAP/PAP 选择、`pap-secrets` |
| 通了但很快断 | LCP 保活超时、信号差、模块掉数据态（如 `+++` 误触发） |
| 抓包看协商 | tcpdump/`pppdump` 看 LCP/IPCP 帧 |

---

## 10. 钩子脚本：/etc/ppp/ip-up 与 /etc/ppp/ip-down

### 10.1 钩子是什么

程序在自己固定流程点上留出"到点就调用外部脚本"的口子，让你不改主程序内核即可插入定制行为。pppd 的实现方式 = **约定路径的可执行脚本**：到点 pppd fork 执行、传固定参数，无需注册。整个 ppp 生态（connect / disconnect / ip-up / ip-down / options / peers）同一套路：**pppd 干协议，具体行为外置脚本化**。

### 10.2 两个脚本的触发点

| 脚本 | pppd 何时调用 |
|------|--------------|
| `/etc/ppp/ip-up` | IPCP 协商完成、接口配上 IP、链路可用时（拨号成功那一刻） |
| `/etc/ppp/ip-down` | 链路终结、拆线时 |

### 10.3 pppd 如何调用（man pppd）

- 以 `/bin/sh` 方式执行该路径的可执行文件；须 root 属主、有执行位
- **位置参数**（脚本里 `$1..$5`）：
  - `$1` 接口名（ppp0）
  - `$2` tty 设备
  - `$3` 链路速率
  - `$4` 本端（协商所得）IP
  - `$5` 对端 IP
  - `$6` `ipparam` 选项值（有则给）
- **ip-up 退出码有意义**：返回非 0，pppd 将终止链路——可当"检查不过不许上线"的闸门
- 多发行版按 run-parts 惯例另扫 `/etc/ppp/ip-up.d/`、`/etc/ppp/ip-down.d/`，逐脚本执行
- 部分 pppd 版本额外通过环境变量传参（如 `PPP_IFACE`）

### 10.4 用法示例

```sh
#!/bin/sh
# /etc/ppp/ip-up
# 参数: 接口 设备 速率 本端IP 对端IP ipparam
IFACE=$1
LOCAL_IP=$4
logger "PPP $IFACE up, ip=$LOCAL_IP"
# 例：通知业务进程网络已就绪
kill -USR1 $(pidof my_app)
```

```sh
#!/bin/sh
# /etc/ppp/ip-down
logger "PPP link down, cleaning up"
```

### 10.5 典型用途

- **ip-up**：刷默认路由/静态路由、起 iptables 规则、启动"必须有网才起"的服务、通知业务层"网通了"、把协商到的 DNS 写入配置
- **ip-down**：回收以上动作——清路由/规则、停服务、标记断线

### 10.6 一句话理解

`ip-up`/`ip-down` = pppd 开给系统管理员的"拨上号了/断线了"两个通知口。拨号结果（接口、IP 等）从位置参数递进来，让你在"链路刚通"和"链路刚断"两个瞬间插入自己的动作。

---

## 11. /etc/ppp 配置目录

### 11.1 定位

- `/etc/ppp` 是 **pppd 的配置目录**，编译期默认路径。内核 PPP 驱动不需要它。
- 目录/文件不存在**不算错**，pppd 照常启动；每个文件"存在才读取、用到才需要"。

### 11.2 标准布局（pppd 默认路径）

```
/etc/ppp/
├── options              # 全局默认选项（任何会话先读）
├── pap-secrets          # PAP 口令：用户名 服务器 密码
├── chap-secrets         # CHAP 口令
├── peers/               # 拨号档案（pppd call <名字>）
│   └── provider
├── chatscripts/         # chat 的 expect/send 脚本
├── ip-up                # 链路 up 钩子（可执行）
├── ip-down              # 链路 down 钩子（可执行）
├── ip-up.d/  ip-down.d/ # 部分发行版 run-parts 子目录
├── ipv6-up / ipv6-down  # IPv6 版本
└── auth-up / auth-down  # 认证通过/失败钩子
```

> 具体路径随发行版/编译参数而变，以 `man pppd` 的 FILES 段或 `strings pppd | grep /etc/ppp` 为准。

### 11.3 何时才需要各文件

| 文件/目录 | 何时需要 |
|-----------|----------|
| `options` | 设全局默认参数时；否则全凭命令行 |
| `peers/` | 用 `pppd call <档案名>` 时 |
| `pap-secrets` / `chap-secrets` | 本端做 PAP/CHAP 认证时 |
| `ip-up` / `ip-down` | 要在链路 up/down 插钩子时（见第 10 节） |
| `chatscripts/` | connect 脚本引用 chat 脚本时 |
| `ipv6-up` / `ipv6-down` | 跑 IPv6 链路时 |

### 11.4 最小可用

纯命令行、不要认证、不要钩子时，可一个文件不建：
```
pppd /dev/ttyS0 38400 noauth defaultroute
```
要管多套配置 → 建 `peers/`；要认证 → `pap-secrets`；要拨通后自动动作 → `ip-up`。

### 11.5 一句话理解

`/etc/ppp` 是 pppd 的"可选配置屋"，不是地基——内核与数据链路不依赖它，缺了只是对应功能没有默认配置可读。

---

## 参考

- RFC 1661 —— Point-to-Point Protocol（PPP）
- RFC 1662 —— PPP in HDLC-like Framing
- RFC 1332 —— The PPP Internet Protocol Control Protocol (IPCP)
- RFC 1994 —— PPP Challenge Handshake Authentication Protocol (CHAP)
- ppp 发行版（pppd）源码，Linux 内核 `Documentation`（ppp_generic）
