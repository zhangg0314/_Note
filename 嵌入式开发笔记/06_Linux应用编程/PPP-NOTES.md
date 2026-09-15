# PPP 协议

[【网络协议详解】——PPP协议（学习笔记）-CSDN](https://blog.csdn.net/HinsCoder/article/details/130781224)

## 1.概述与定位

PPP（Point-to-Point Protocol）为点对点链路层协议，运行于字节流式的点对点物理通道之上（串口、Modem 拨号、ISDN、光纤直连、蜂窝拨号等），承担以下职责：

1. 将字节流封装为具有明确边界的帧，承载网络层协议（主要为 IP）
2. 建立、配置、维护与终结链路
3. 在链路上协商网络层参数（IP 地址、DNS 等）

协议族定位：

```txt
应用层（HTTP/业务）
传输层（TCP/UDP）
网络层（IP）          ← IP 包由 PPP 封装
数据链路层：PPP
物理层（串口/Modem/无线基带）
```

PPP 对应 OSI 第二层，同类链路层协议另有 HDLC、以太网。PPP 不限定物理介质形态，仅要求介质能够传输字节。PPP 亦可承载 IP 之外的协议（如 IPX），但该用法现已趋于淘汰。

## 2.解决的问题

裸串口仅提供一条字节通道，若直接承载 IP，将面临下列问题；PPP 针对每一项给出对应机制：

| 问题 | PPP 的解决 |
|------|-----------|
| 字节流无包边界，接收方无法判定数据段结束位置 | 帧格式：以标志字节 `0x7E` 定界，辅以长度/校验（RFC 1662） |
| 双方参数（MRU、认证方式等）缺乏协商机制 | 由 LCP 协商 |
| 无认证机制，任意对端均可接入 | PAP / CHAP / EAP 可选认证 |
| 网络层参数（本端 IP、对端网关 IP、DNS）需动态分配 | 由 NCP（通常为 IPCP）协商分配 |
| 物理链路异常中断无法感知 | LCP 保活（Echo-Request）与终止流程 |
| 链路上需同时承载多种网络层协议 | 协议字段多路复用 |

PPP 的价值并不限于承载 IP——早期点对点链路协议（如 SLIP）同样具备该能力。其核心价值在于提供协商、认证与配置能力，即链路建立阶段的会话机制。

## 3.PPP帧格式

RFC 1662 定义的类 HDLC 帧结构：

```
7E | FF | 03 | 协议(2B) | 信息(变长) | FCS(2B) | 7E
标志 地址 控制  协议号      载荷         校验     标志
```

- **标志 `0x7E`**
  帧边界。载荷中若出现 `0x7E`，须进行字节填充转义（`0x7D` 加异或 `0x20`），以保证标志字节唯一
- **地址 `0xFF` / 控制 `0x03`**
  固定值。PPP 为点对点链路，无需链路层寻址，该二字段沿用 HDLC 帧格式保留
- **协议号**（多路复用的依据）：
  - `0x0021` IPv4
  - `0xc021` LCP（链路控制）
  - `0xc023` PAP 认证 / `0xc223` CHAP 认证
  - `0x8021` IPCP（IPv4 网络控制）
  - `0x8057` / `0x8107` 压缩类，`0x003d` 多链路等
- **信息**
  网络层报文或协商报文
- **FCS**
  帧校验，校验失败即丢弃

## 4.子控制协议

PPP 的链路生命周期由三类控制协议驱动，另加数据阶段，共计四个部分。

### 1.LCP

Link Control Protocol，链路控制协议

- 协议号 `0xc021`，于建链阶段首先运行
- 协商 MRU、认证协议、asyncmap、魔数（用于环回检测）等
- 链路质量报告、Echo-Request/Reply 保活
- Terminate-Request 终止链路

### 2.认证协议

- **PAP**（`0xc023`）
  以明文传输用户名与密码，实现简单
- **CHAP**（`0xc223`）
  采用挑战—应答机制，口令不以明文上线
- **EAP**
  可扩展认证（支持证书等）
- 是否认证、采用何种方式，均在 LCP 阶段先行协商；认证失败则链路终止

### 3.NCP

每种网络层协议对应一个 NCP。承载 IPv4 时使用 **IPCP**（`0x8021`），协商内容：

- 本端 IP 地址（可自动分配）
- 对端 IP（通常为运营商或对端路由器地址）
- DNS 服务器（`usepeerdns` 场景下关键）

NCP 完成后进入数据阶段，方可实际传输 IP 报文。

### 4.数据阶段

协议字段直接填写网络层协议号（如 `0x0021`），信息字段即为 IP 报文。

## 5.链路状态机

RFC 1661 定义的状态迁移：

```
Dead(物理不通)
  ↓ 物理层 up
Establishing      ← 运行 LCP，协商参数
  ↓ LCP 完成
Authenticating    ← 认证（可选）
  ↓ 认证通过 / 无需认证
Network           ← 运行 NCP/IPCP，获取 IP
  ↓ IPCP 完成
Open              ← 数据阶段，传输 IP
  ↓ 断开 / 保活失败
Terminating → Dead
```

故障排查时，可依据该状态迁移图判断链路停滞于哪一阶段。

---

# Linux 实现

## 1.Linux的PPP组成

| 部分 | 定位 | 作用 |
|------|------|------|
| **数据面** | 内核 `ppp_generic` + `ppp_async`（`/dev/ppp` 及 `ppp0` 网卡） | 逐包 HDLC 组帧/解帧、协议分发，应用 socket 经由 `ppp0` 网卡 |
| **控制面** | `pppd` 守护进程（用户态） | 与内核 PPP 驱动协作建立并维护链路，协商两端 IP，运行 LCP/IPCP/认证状态机 |
| **建立物理链路** | `chat` 或自定义 connect 程序 | 进入 PPP 前向 Modem 发送 AT 指令建立承载，使链路进入数据态，使模块进入外部协议栈 |

- pppd 守护进程不参与逐包数据搬运，其在链路建立后，其职责限于控制面（保活与重协商）
- 协商控制帧（LCP/IPCP）本质亦为 PPP 帧，同样经内核封装，其应答逻辑位于 pppd
- 数据面仅负责帧的封装方式与转发方向，不参与协商决策
- 调用形式：`pppd [选项]`

## 2.PPPD 用法

### 1.调用方式

```
pppd [设备] [波特率] [选项...]
```

- **设备名**与**波特率**为位置参数，其余均以 `关键字 [值]` 形式给出
- 选项**不使用 `-` 前缀**，先后顺序不影响结果
- 布尔选项以 `no` 前缀取反：`auth`↔`noauth`、`vj`↔`novj`、`ccp`↔`noccp`
- 设备名不含 `/` 时，自动补 `/dev/` 前缀（`ttyS0` 等价于 `/dev/ttyS0`）

下例为一条完整的拨号命令：

```sh
pppd /dev/ttyUSB0 115200 \                  # 位置参数：设备与速率
     connect "/usr/sbin/chat -f /etc/ppp/chatscripts/gprs" \   # 建链脚本，负责 AT 拨号
     disconnect "/usr/sbin/chat -f /etc/ppp/chatscripts/disc" \  # 断链脚本
     noauth \                               # 不要求对端认证本端
     defaultroute \                         # IPCP 完成后添加默认路由
     usepeerdns \                           # 接受对端下发的 DNS
     persist holdoff 10                     # 链路终止后等待 10 s 重拨
```

同一组选项可书写于三处，效果等价：

| 书写位置 | 适用 |
|----------|------|
| 命令行 | 临时调试 |
| `/etc/ppp/options` | 本机所有 pppd 会话的公共项 |
| `/etc/ppp/peers/<名>`，以 `pppd call <名>` 调用 | 成套拨号配置；**可包含特权选项**（如 `name`），`~/.ppprc` 不具备该能力 |

三处写法的等价对照：

```sh
# 示例一：全部选项书写于命令行
pppd /dev/ttyUSB0 115200 connect "/usr/sbin/chat -f /etc/ppp/chatscripts/gprs" \
     name cmnet noauth defaultroute usepeerdns persist
```

```sh
# 示例二：全部选项置于 peers 档案，命令行仅保留调用
# /etc/ppp/peers/gprs-wave

/dev/ttyUSB0
115200
connect "/usr/sbin/chat -f /etc/ppp/chatscripts/gprs"
name cmnet
noauth
defaultroute
usepeerdns
persist

pppd call gprs-wave          # 与上述配置等价
```

同一选项于多处给出时，**按来源优先级判定，与书写先后无关**：

命令行优先于各配置文件（1）；`/etc/ppp/options.ttyname` 虽在命令行之后读入，亦不能覆盖命令行。例外为 `connect`、`disconnect`、`init`、`pty`、`welcome` 与设备名这六个特权选项：一经 root 在配置文件中设定，非 root 用户自命令行覆盖时将报错 `cannot be overridden`，pppd 随即以退出码 2 终止。

### 2.2 典型用法

| 场景 | 命令 |
|------|------|
| 两机串口直连，不启用认证 | `pppd /dev/ttyS0 38400 noauth` |
| 直连并接入 IP 网络 | `pppd /dev/ttyS0 38400 noauth defaultroute usepeerdns` |
| 蜂窝拨号（chat 建链） | `pppd call gprs-wave` |
| 蜂窝拨号（自定义 connect 程序） | `pppd /dev/ttyUSB0 115200 connect "dial.sh c" disconnect "dial.sh d" noauth defaultroute usepeerdns persist` |
| 本端须向对端完成 PAP 认证 | 附加 `user cmnet password cmnet` |
| 要求对端以 CHAP 认证 | 附加 `auth require-chap refuse-pap` |
| 固定网口编号（指定为 ppp3） | 附加 `unit 3` |
| 无人值守，链路终止后自动重拨 | 附加 `persist holdoff 10 maxfail 0` |
| 前台调试，输出全部协商报文 | 附加 `nodetach debug` |
| 仅校验配置语法，不实际拨号 | 附加 `dryrun` |
| 对端无响应（模块仍在线而链路已断）时自动断开 | 附加 `lcp-echo-interval 30 lcp-echo-failure 4` |

### 2.3 常用选项

默认值以本仓库 `ppp-2.4.5` 源码为准。

| 选项 | 默认 | 用途 |
|------|------|------|
| `<设备> <速率>` | — | 位置参数，必须给出 |
| `connect "..."` | — | 建链脚本。pppd 将已打开的串口设备作为其 stdin/stdout 传入；退出码非 0 即建链失败（对应 pppd 退出码 8） |
| `disconnect "..."` | — | 断链脚本，链路断开时调用 |
| `noauth` | 要求认证 | 不要求对端认证本端 |
| `auth` / `require-chap` / `require-pap` | — | 要求对端认证；`refuse-pap`、`refuse-chap` 拒绝对端使用某方式 |
| `name <名>` | 主机名 | 本端认证名（特权选项），确定查 secrets 文件时所匹配的行 |
| `user <名>` / `password <口令>` | — | 本端向对端证明身份；`password` 将出现于 `ps` 输出，仅建议调试时使用 |
| `noipdefault` | 由主机名推 | 本端地址改由对端经 IPCP 分配 |
| `defaultroute` | 关闭 | IPCP 完成后添加默认路由，链路断开时自动删除 |
| `usepeerdns` | 关闭 | 接受对端下发的 DNS，写入 `/etc/ppp/resolv.conf`，并经 `DNS1`/`DNS2` 传给 ip-up |
| `ipparam <串>` | — | 作为第 6 个参数传递给 ip-up |
| `mtu n` / `mru n` | 1500 | 收发最大帧长；`mru` 取值范围 128–16384，慢速链路可设为 296 |
| `asyncmap <hex>` | 0 | 32 位 ACCM，bit 0 对应字符 `0x00`；某位置 1 表示该控制字符须转义 |
| `escape xx,yy` | — | 强制转义指定字符（十六进制列表）；`0x20`–`0x3f` 与 `0x5e` 不可转义 |
| `persist` | 关闭 | 链路终止后重新打开设备 |
| `holdoff n` | 30 s | 重启前等待时长（秒），仅在 `persist` 或 `demand` 下生效 |
| `maxfail n` | 10 | 连续失败 n 次后退出；取 0 表示不限 |
| `idle n` | 关闭 | 空闲达 n 秒后断开链路 |
| `unit n` | 自动分配 | 强制指定网口编号，如需 ppp3 则写 `unit 3` |
| `lcp-echo-interval n` | 0（不发送） | 每 n 秒发送一次 Echo-Request 探测对端存活 |
| `lcp-echo-failure n` | — | 连续 n 次无应答即断链，须配合非零的 `lcp-echo-interval` |
| `lcp-restart n` | 3 s | LCP 报文重传间隔；同类选项 `lcp-max-configure` 默认 10 次、`ipcp-restart` 默认 3 s |
| `nodetach` | 转入后台 | 保持前台运行，不转入后台 |
| `debug` | 关闭 | 记录全部控制报文（LCP/PAP/CHAP/EAP/IPCP），经 syslog `daemon` facility 输出 |
| `dryrun` | — | 仅解析并输出生效的选项值后退出，用于校验配置语法 |

完整选项表参见 `man pppd`。需注意 `chap-timeout`、`replacedefaultroute`、`defaultroute-metric` 以及作为选项的 `ifname` 均为 2.4.6 及以后版本引入，本仓库所用 2.4.5 中不存在；写入配置将报 `unrecognized option`，并以退出码 2 结束。

### 2.4 选项文件

读入顺序：

```
/etc/ppp/options                  # 全局
~/.ppprc                          # 当前用户
命令行
/etc/ppp/options.ttyname          # 按设备名，最后由 tty 层读取
```

- 先后顺序不影响生效结果，判定规则见 2.1
- `options.ttyname` 的文件名由设备名导出：去掉前导 `/dev/`，其余斜杠替换为点号（`/dev/mux0` → `/etc/ppp/options.mux0`）
- 解析规则：以空白分隔词；双引号保留词内空白；反斜杠转义下一字符；`#` 起至行尾为注释

## 3. /etc/ppp 配置目录

### 3.1 定位

- `/etc/ppp` 为 pppd 的配置目录，其路径在编译期确定
- 目录或文件缺失不视为错误，pppd 仍可正常启动；各文件遵循"存在即读取、按需创建"的原则

### 3.2 默认路径

```shell
/etc/ppp/
├── options              # 全局默认选项（任何会话首先读取）
├── options.ttyname      # 按终端名区分的选项文件
├── pap-secrets          # PAP 口令：用户名 服务器 密码
├── chap-secrets         # CHAP 口令
├── srp-secrets          # SRP 口令
├── peers/               # 拨号档案（pppd call <名字>）
│   └── provider
├── chatscripts/         # chat 的 expect/send 脚本
├── auth-up / auth-down  # 认证通过 / 失败钩子
├── ip-pre-up            # IP 接口已配地址但尚未 up（pppd 阻塞等待其完成）
├── ip-up                # 链路 up 钩子（可执行）
├── ip-down              # 链路 down 钩子（可执行）
├── ip-up.d/  ip-down.d/ # 部分发行版 run-parts 子目录
├── ipv6-up / ipv6-down  # IPv6 版本
└── ipx-up / ipx-down    # IPX 版本
```

> 具体路径随发行版及编译参数而异，以 `man pppd` 的 FILES 段或 `strings pppd | grep /etc/ppp` 为准。

### 3.3 何时需要各文件

| 文件/目录 | 何时需要 |
|-----------|----------|
| `options` | 需设置全局默认参数时；未创建时则完全依赖命令行 |
| `peers/` | 使用 `pppd call <档案名>` 时 |
| `pap-secrets` / `chap-secrets` / `srp-secrets` | 本端执行相应认证时 |
| `ip-up` / `ip-down` | 需在链路 up/down 时插入钩子（见 5 节） |
| `chatscripts/` | connect 脚本引用 chat 脚本时 |
| `ipv6-up` / `ipv6-down` | 承载 IPv6 链路时 |

### 3.4 最小可用

在无需认证与钩子的纯命令行场景下，可不创建任何文件：

```
pppd /dev/ttyS0 38400 noauth defaultroute
```

需管理多套配置时创建 `peers/`；需认证时配置 `pap-secrets`；需在链路建立后自动执行动作时配置 `ip-up`。

### 3.5 运行时文件

| 文件 | 说明 |
|------|------|
| `/var/run/pppn.pid` 或 `/etc/ppp/pppn.pid` | 按接口编号的 PID 文件 |
| `/var/run/ppp-name.pid` 或 `/etc/ppp/ppp-name.pid` | `linkname` 指定的 PID 文件 |
| `/var/run/pppd2.tdb` | 进程/接口/链路数据库，多链路匹配用 |
| `/etc/ppp/{pap,chap,srp}-secrets` | 须 root 属主且他人不可读写，否则记录警告 |
| `~/.ppp_pseudonym` | SRP 假名文件 |

## 4. connect / disconnect 机制

pppd 自身不执行拨号动作，而是将建链过程交由外部可执行程序完成：

- `connect "..."`
  由 pppd fork 该程序（经 `/bin/sh -c` 执行），并将自身已打开的串口设备作为子进程的 stdin/stdout 传入；子进程负责发送 AT 指令与拨号，成功时以退出码 0 返回，pppd 随即在同一设备上开始 PPP 协商
- `disconnect "..."`
  链路断开时触发，发送 AT 指令挂断并将模块恢复至命令态

经典实现为 `chat`（依据 expect/send 脚本工作）；任何具备同等功能的程序均可替代，行为等价。

协商时序如下：connect / init 脚本首先执行；`connect-delay`（默认 1000 ms）等待对端发出有效报文；随后 pppd 发送首个 LCP 帧；IPCP/CCP/IPv6CP/IPXCP 更晚启动，此时触发 `ip-up` 系列脚本。

## 5. 脚本与钩子

### 5.1 钩子含义

hook 一词源于 Windows 消息钩子机制：将自定义处理函数挂入系统消息流，以截获、加工并放行消息。其核心语义为：将外部实现挂接至既有流程之上。

程序在自身的固定流程节点预留调用外部脚本的接口，使定制行为无需修改主程序即可插入。pppd 的实现方式为**约定路径的可执行脚本**：到达既定节点时由 pppd fork 执行并传入固定参数，无需注册。整个 ppp 生态（connect / disconnect / ip-up / ip-down / options / peers）均采用同一模式：pppd 负责协议，具体行为外置为脚本。

### 5.2 脚本一览

| 脚本 | 触发时机与参数 |
|------|----------------|
| `/etc/ppp/auth-up` | 认证完成后：`接口名 对端名 用户名 tty设备 速率`；`noauth` 时跳过 |
| `/etc/ppp/auth-down` | 参数同 auth-up |
| `/etc/ppp/ip-pre-up` | 接口已获得地址但处于 down 状态；参数同 ip-up；pppd 阻塞直至其结束 |
| `/etc/ppp/ip-up` | IPCP 完成、链路可用：`接口名 tty设备 速率 本端IP 对端IP ipparam` |
| `/etc/ppp/ip-down` | 链路终结：参数同 ip-up |
| `/etc/ppp/ipv6-up` / `ipv6-down` | IPv6 对应脚本 |
| `/etc/ppp/ipx-up` / `ipx-down` | IPX 对应脚本，12 个位置参数：接口名、tty 设备、速率、网络号、本/对端 IPX 节点地址、本/对端 IPX 路由协议（`NONE`/`RIP`/`NLSP`/`RIP NLSP`）、本/对端 IPX 路由器名、ipparam、pppd PID |

### 5.3 执行环境与变量

- 脚本以 root 身份运行；除 `ip-pre-up` 外均**不被等待**；stdin/stdout/stderr 重定向至 `/dev/null`，环境变量近乎为空
- 通用变量：`DEVICE`、`IFNAME`、`IPLOCAL`、`IPREMOTE`（后两者仅在 IPCP 完成后存在）、`PEERNAME`（仅当对端通过认证）、`SPEED`、`ORIG_UID`、`PPPLOGNAME`
- `ip-down` / `auth-down` 额外提供：`CONNECT_TIME`、`BYTES_SENT`、`BYTS_RCVD`、`LINKNAME`、`CALL_FILE`、`DNS1`、`DNS2`
- 注意：**connect / disconnect 类脚本的 stdio 为串口设备**（由 pppd 派生时 dup2 至 fd 0/1），与上述 up/down 钩子脚本的 `/dev/null` 处理不同

### 5.4 调用方式与参数

- 以 `/bin/sh` 方式执行该路径下的可执行文件；须为 root 属主且具备执行权限
- `ip-up` 位置参数（脚本内 `$1..$6`）：
  - `$1` 接口名（ppp0）
  - `$2` tty 设备
  - `$3` 链路速率
  - `$4` 本端（协商所得）IP
  - `$5` 对端 IP
  - `$6` `ipparam` 选项值（配置时传入）
- **ip-up 退出码具有语义**
  返回非 0 时 pppd 将终止链路，可用作准入校验点
- 多数发行版按 run-parts 惯例另行扫描 `/etc/ppp/ip-up.d/`、`/etc/ppp/ip-down.d/` 并逐个执行

### 5.5 用法示例

```sh
#!/bin/sh
# /etc/ppp/ip-up
# 参数: 接口 设备 速率 本端IP 对端IP ipparam
IFACE=$1
LOCAL_IP=$4
logger "PPP $IFACE up, ip=$LOCAL_IP"
# 通知业务进程网络已就绪
kill -USR1 $(pidof my_app)
```

```sh
#!/bin/sh
# /etc/ppp/ip-down
logger "PPP link down, cleaning up"
```

### 5.6 典型用途

- **ip-up**
  刷新默认路由与静态路由、下发 iptables 规则、启动依赖网络的服务、通知业务层网络就绪、将协商所得 DNS 写入配置
- **ip-down**
  回收上述动作：清理路由与规则、停止服务、标记断线

## 6. 故障排查

| 现象                        | 排查方向                                              |
| --------------------------- | ----------------------------------------------------- |
| 拨号后未出现 `ppp0`         | connect 是否以退出码 0 返回；AT 是否已到达 CONNECT    |
| 有 `ppp0` 但无 IP / 未 up   | IPCP 是否完成；`noauth`/口令配置是否正确              |
| 停滞于 LCP                  | 对端未响应 LCP；asyncmap/转义不匹配；串口速率不一致   |
| 认证失败                    | 用户名/口令、CHAP/PAP 选择、`pap-secrets` 配置        |
| 连通后迅速断开              | LCP 保活超时、信号质量差、模块退出数据态（如误发 `+++`） |
| 需分析协商过程时            | 使用 tcpdump/`pppdump` 抓取 LCP/IPCP 帧               |

### 6.1 pppd 退出码

| 码 | 含义 |
|----|------|
| 0 | 已后台运行；或连接建立后由对端请求终止 |
| 1 | 立即致命错误（关键系统调用失败、无可用内存） |
| 2 | 选项处理错误（如互斥选项） |
| 3 | 非 setuid-root 且调用者非 root |
| 4 | 内核不支持 PPP（无驱动或无法加载） |
| 5 | 收到 SIGINT / SIGTERM / SIGHUP |
| 6 | 串口加锁失败 |
| 7 | 串口无法打开 |
| 8 | connect 脚本失败（返回非 0） |
| 9 | `pty` 指定命令无法执行 |
| 10 | PPP 协商失败，无网络层协议进入运行态 |
| 11 | 对端认证失败或被拒绝 |
| 12 | 链路建立后因空闲断开 |
| 13 | 链路建立后因连接超时终止 |
| 14 | 正在协商回拨，来电即将接入 |
| 15 | 对端停止响应 Echo 请求 |
| 16 | Modem 挂断 |
| 17 | 检测到串口环回，协商失败 |
| 18 | init 脚本返回非 0 |
| 19 | 本端未能通过向对端认证 |

## 7. 信号

| 信号 | 行为 |
|------|------|
| `SIGINT` / `SIGTERM` | 经 LCP 拆链、恢复串口设置后退出 |
| `SIGHUP` | 行为同上，并关闭串口设备；配置 `persist` 或 `demand` 时于 holdoff 延时后重新打开并拨号，否则退出；同时立即终止进行中的 holdoff |
| `SIGUSR1` | 切换 `debug` 选项 |
| `SIGUSR2` | 触发压缩重协商（适用于因解压致命错误而禁用压缩后） |

## 8. 日志与诊断

- pppd 通过 syslog 输出，facility 为 `LOG_DAEMON`；落盘位置由 `/etc/syslog.conf`（或 rsyslog 配置）决定
- 启用 `debug` 将记录全部控制报文内容（LCP、PAP、CHAP、EAP、IPCP）

## 9. 安全模型

- 未通过认证的对端，仅可使用系统中不存在路由的 IP 地址；在配置了默认路由的主机上，`auth` 实质为默认行为
- 特权选项仅接受来自 root 控制的来源：`/etc/ppp/options`、经由 `call` 读入的 peers 文件，或 pppd 本身以 root 运行
- 设备打开时的凭据取决于设备名的来源

---

# 蜂窝（GPRS/3G/4G）

## 1. 链路建立

- 主机向模块发送 AT 数据呼叫指令
  `ATD*99***1#`（GSM/UMTS 的 PPP 型 PDP 数据呼叫；CDMA 常用 `ATDT#777`）
- 模块注册网络并建立承载后进入**数据模式**（PPP 透传）
- 主机随后运行 pppd
  经 LCP/IPCP 协商获取 IP，得到 `ppp0` 接口与默认路由

## 2. 数据去向

PPP 帧的终点并非远端服务器，而是运营商网关 GGSN/PGW：

```
主机 ←串口PPP帧→ 模块 ←空口无线帧(PDCP/RLC/MAC，PPP 仅为其载荷)→ 运营商 GGSN/PGW
  PPP 在 GGSN 解封装、还原 IP，由此接入骨干网
  → 骨干网/公网 → 远端服务器（服务器接收到的是普通 IP 报文）
```

## 3. 模块的两种角色

1. **PPP 透传模式**
   模块仅作透明通道，PPP 实现完全位于主机侧，主机使用 `ppp0` 接口与自身 IP 协议栈
2. **内置协议栈 / AT socket**
   模块内部完成 TCP/IP 封装，主机不使用 PPP；串口上传输的是裸应用数据，模块按给定远端地址自行建立 socket

---

# AT 指令

## 1. 定位

AT 指令为 DTE（Data Terminal Equipment，数据终端设备，此处指主机）控制 DCE（Data Circuit-terminating Equipment，数据电路终接设备，此处指模块）的命令通道，源自 1981 年 Hayes 命令集，后续由 3GPP TS 27.007 等规范标准化。名称取自 ATtention，每条指令以 `AT` 起首。其物理承载与 PPP 相同，均为同一条串口，区别在于该串口所处的状态：

| 状态 | 串口上传输的内容 | 模块行为 | 主机侧程序 |
|------|------------------|----------|------------|
| **命令态** | ASCII 命令行，以 `\r` 结尾 | 解析并执行 AT 指令，按行返回结果码 | `chat`、`pppdialer` 等 connect 程序 |
| **数据态** | 任意字节流（PPP 帧） | 透明转发至无线侧，**不再解析 AT** | `pppd`（LCP/IPCP） |

两种状态互斥。模块处于数据态时，AT 指令失去意义，且被当作普通数据发送。

## 2. 与 PPP 的衔接

PPP 状态机的起点为 `Dead`，其迁移至 `Establishing` 的前提是物理层已就绪（见"PPP 协议 / 5. 链路状态机"）。蜂窝场景下，"物理链路"指模块至基站的一条 PDP（Packet Data Protocol，分组数据协议）上下文承载，该承载并非常备，须由主机指令模块建立。AT 即用于完成这一建立过程：

| 阶段 | 典型指令 | 作用 |
|------|----------|------|
| 自检 | `ATE0`、`AT+CPIN?`、`AT+CSQ`、`AT+CREG?` | 关闭回显、确认 SIM 就位、查询信号质量、确认网络注册状态 |
| 配置 | `AT+CGDCONT=1,"IP","<APN>"` | 定义 PDP 上下文，指定 APN 与承载类型 |
| 拨号 | `ATD*99***1#` | 令模块向网络发起 PDP 激活 |
| 挂断 | `+++`、`ATH`、`AT+CGACT=0,1` | 退回命令态并释放承载 |

时序关系：

```
init 脚本（初始化串口）
  → connect 脚本：命令态下发送 AT，ATD 成功后模块切入数据态
  → connect 脚本以退出码 0 结束，交出串口
  → pppd 于同一串口上发送首个 LCP 帧（经 connect-delay 等待）
  → IPCP 完成，ppp0 可用
```

关键交接点在拨号指令：`ATD*99***1#` 返回 `CONNECT` 表示承载已建立、模块已进入数据态，此后串口内容由 PPP 接管。

反向切换需注意：数据态下发送转义序列 `+++`（前后各需约 1 s 静默）可使模块退回命令态，但此举将中断 PPP 链路。部分模块改用 DTR 信号线下降沿完成同一动作。

## 3. 为何由外部程序发送

pppd 不发送 AT 指令，其与外部程序的接口仅为"一个命令行字符串加一个退出码"。原因有三：

1. **接口抽象**：`connect` 的语义是"pppd 启动前运行该程序，退出码 0 表示成功"。pppd 不解析该程序的内容，因而同一机制可覆盖直连串口（无需 AT）、ISDN、PPPoE 等全部场景
2. **指令方言**：AT 指令集存在大量厂商私有扩展，返回值格式亦不统一。若由 pppd 内置，则需在 pppd 中维护各厂商模块的兼容逻辑
3. **历史沿革**：AT 早于 PPP 出现，pppd 沿用了既有的对话工具 `chat`（依 expect/send 脚本工作），而非自行实现。（此条为设计沿革推断，未在代码中直接验证）

由此形成分工：pppd 负责 PPP 协议本身，AT 交互外置为可替换的 connect 程序。

## 4. 常用指令

### 4.1 基本控制

| 指令 | 作用 |
|------|------|
| `AT` | 检测模块响应，正常返回 `OK` |
| `ATE0` / `ATE1` | 关闭 / 打开命令回显 |
| `AT+CMEE=1` | 错误码采用数字形式；`=2` 为文本形式，`=0` 为关闭 |
| `AT+CGMI` / `AT+CGMM` / `AT+CGMR` | 查询厂商 / 型号 / 固件版本 |

### 4.2 状态查询

| 指令 | 作用 |
|------|------|
| `AT+CPIN?` | SIM 状态，`READY` 表示就绪 |
| `AT+CSQ` | 信号质量，返回接收信号强度（0–31，99 表示未知）与误码率 |
| `AT+COPS?` | 当前注册的运营商 |

### 4.3 网络注册

| 指令 | 作用 |
|------|------|
| `AT+CREG?` | 电路域注册状态：0 未注册、1 已注册（本地）、2 正在搜索、3 注册被拒、4 未知、5 已注册（漫游） |
| `AT+CGREG?` | 分组域（GPRS）注册状态，编码同上 |

### 4.4 PDP 与拨号

| 指令 | 作用 |
|------|------|
| `AT+CGDCONT=1,"IP","<APN>"` | 定义 1 号 PDP 上下文，承载类型 IP，指定 APN |
| `ATD*99***1#` | 请求 1 号上下文的数据业务；`ATD*99#` 为省略形式，默认使用 1 号上下文 |
| `ATDT#777` | CDMA 制式常用拨号指令 |

### 4.5 挂断与状态切换

| 指令 | 作用 |
|------|------|
| `+++` | 转义序列，由数据态退回命令态（前后各需约 1 s 静默） |
| `ATO` | 由命令态返回数据态 |
| `ATH` | 挂断 |
| `AT+CGACT=0,1` | 去激活 1 号 PDP 上下文，释放承载 |

## 5. 应答与错误

模块对每条指令返回结果码。正常情况返回 `OK`；涉及链路建立时返回 `CONNECT`；异常情况返回下列之一：

| 结果码 | 含义 |
|--------|------|
| `ERROR` | 指令语法错误或不支持 |
| `NO CARRIER` | 承载建立失败或已中断 |
| `BUSY` | 对端忙 |
| `NO ANSWER` | 对端无应答 |
| `NO DIALTONE` | 无拨号音（主要用于固定电话 Modem） |
| `+CME ERROR` | 设备类错误 |
| `+CMS ERROR` | 短信类错误 |

connect 程序须逐条等待期望的应答，并对上述异常码作出判定。回应等待需设超时，否则模块无响应时程序将永久阻塞。

## 6. sg698项目实现

本项目以自研程序 `pppdialer` 替代通用工具 `chat`，二者职责对等：均在命令态下驱动模块，成功后退出并交还串口。差异如下：

| 方面 | `chat` | `pppdialer` |
|------|--------|-------------|
| 指令来源 | expect/send 脚本，运行时读取 | 型号指令表，编译期内置（`dial.c`） |
| 型号适配 | 由脚本编写者自行处理 | 按 `/etc/ppp/moduleid` 内容选择指令表（`dial.c:163`，`dial.h:12`） |
| 超时机制 | 由脚本参数指定 | 固定采用 `alarm` 定时（`maind.c:248`） |
| 异常判定 | 由脚本 abort 项指定 | 内置异常串集合（`maind.c:169-172`）：`NO ANSWER`、`NO CARRIER`、`NO DIALTONE`、`+CME ERROR` 等 |

型号指令表以结构体数组形式描述每一步的发送内容与期望应答。以 GSM 07.07 通用表为例（`dial.c:56+`）：

```
发送序列: AT, ATE0, AT+CSQ, AT+CPIN?, AT+CREG?, ATD*99***1#
期望应答: OK, OK,   OK,     OK,       OK,       CONNECT
```

具体实现要点：

1. **读写均通过标准输入输出**：`put_string()` 向 fd 1 写入指令并补 `\r`（`maind.c:211`）；`get_string()` 自 fd 0 读取并逐字符比对期望串（`maind.c:223`）。该两个 fd 由 pppd 派生时指向串口设备
2. **拨号成功即退出**：执行至 `ATD*99***1#` 并取得 `CONNECT` 后，程序返回 0 并退出（`dial.c:2083`），串口随即交由 pppd 用于 LCP/IPCP。数据态下该程序不再介入
3. **厂商私有指令**：如 Neoway 模块的认证指令 `AT+XGAUTH=1,1,"<用户名>","<口令>"`（`dial.c:949`），以及基于高通平台的 NV 项读写 `AT+QNVFR` / `AT+QNVFW`（`dial.c:1369`）。此类指令不在 3GPP 标准范围内，是必须外置为独立程序的直接原因
4. **断链序列**：断链时执行挂断指令（表中 `ATH`），使模块退回命令态

另需注意 AT 与终端时代的遗留影响：即便链路已进入纯 PPP 数据态，中间环节仍可能吞掉特定控制字符，故配置中常见 `asyncmap 0xa0000` 与 `escape FF`（见树内样例 `ppp/peers/gprs-wave-sh`）。`0xa0000` 对应 `0x11`（XON）与 `0x13`（XOFF），`FF` 沿用终端设备的传统约定。

---

# 缩略语

## 1. 协议与链路

| 缩写 | 全称 | 说明 |
|------|------|------|
| PPP | Point-to-Point Protocol | 点对点协议 |
| HDLC | High-level Data Link Control | 高级数据链路控制，PPP 帧格式的来源 |
| SLIP | Serial Line Internet Protocol | 串行线路网际协议，PPP 之前身 |
| LCP | Link Control Protocol | 链路控制协议 |
| NCP | Network Control Protocol | 网络控制协议，各类 NCP 的总称 |
| IPCP | Internet Protocol Control Protocol | IPv4 的网络控制协议 |
| IPv6CP | IPv6 Control Protocol | IPv6 的网络控制协议 |
| IPXCP | Internetwork Packet Exchange Control Protocol | IPX 的网络控制协议 |
| CCP | Compression Control Protocol | 压缩控制协议 |
| PAP | Password Authentication Protocol | 口令认证协议，口令以明文传输 |
| CHAP | Challenge Handshake Authentication Protocol | 挑战握手认证协议 |
| EAP | Extensible Authentication Protocol | 可扩展认证协议 |
| MPPE | Microsoft Point-to-Point Encryption | 微软点对点加密 |
| MPPC | Microsoft Point-to-Point Compression | 微软点对点压缩 |
| MP | Multilink Protocol | 多链路协议 |
| FCS | Frame Check Sequence | 帧校验序列 |
| ACCM | Async-Control-Character-Map | 异步控制字符映射表，即 `asyncmap` |
| MRU | Maximum Receive Unit | 最大接收单元 |
| MTU | Maximum Transmit Unit | 最大发送单元 |
| NAK | Negative Acknowledgment | 否定应答，LCP/IPCP 协商用语 |
| LQR | Link Quality Report | 链路质量报告 |
| PPPoE | PPP over Ethernet | 以太网上的 PPP |
| IP | Internet Protocol | 网际协议 |
| IPX | Internetwork Packet Exchange | 网间分组交换，Novell 协议 |
| DNS | Domain Name System | 域名系统 |
| OSI | Open Systems Interconnection | 开放系统互连参考模型 |
| ISDN | Integrated Services Digital Network | 综合业务数字网 |
| RFC | Request for Comments | IETF 文档编号体系 |

## 2. 蜂窝与模块

| 缩写 | 全称 | 说明 |
|------|------|------|
| AT | ATtention | 模块命令集，源自 Hayes |
| DTE | Data Terminal Equipment | 数据终端设备，主机侧 |
| DCE | Data Circuit-terminating Equipment | 数据电路终接设备，模块侧 |
| DTR | Data Terminal Ready | 数据终端就绪，串口控制线 |
| APN | Access Point Name | 接入点名称 |
| PDP | Packet Data Protocol | 分组数据协议；PDP 上下文即一条数据承载 |
| GPRS | General Packet Radio Service | 通用分组无线业务 |
| GSM | Global System for Mobile Communications | 全球移动通信系统 |
| UMTS | Universal Mobile Telecommunications System | 通用移动通信系统（3G） |
| CDMA | Code Division Multiple Access | 码分多址 |
| SIM | Subscriber Identity Module | 用户身份模块 |
| GGSN | Gateway GPRS Support Node | GPRS 网关支持节点 |
| PGW | Packet Data Network Gateway | 分组数据网关（4G 核心网，亦称 P-GW） |
| PDCP | Packet Data Convergence Protocol | 分组数据汇聚协议，空口协议栈之一 |
| RLC | Radio Link Control | 无线链路控制 |
| MAC | Medium Access Control | 介质访问控制 |
| NV | Non-Volatile | 模块非易失存储项 |

## 3. 系统与工具

| 缩写 | 全称 | 说明 |
|------|------|------|
| PID | Process ID | 进程号 |
| UUCP | Unix-to-Unix Copy | 早期 Unix 拨号网络套件；`lock` 沿用其锁文件风格 |
| SRP | Secure Remote Password | 安全远程口令协议，pppd 支持但现已少用 |
| syslog | system log | 系统日志机制 |

## 4. AT 指令构成

`AT` 之后的部分由后缀构成。基础指令为单个字母，扩展指令以 `+` 起首。

| 后缀 | 全称 / 含义 | 示例 |
|------|-------------|------|
| `D` | Dial，拨号 | `ATD*99***1#` |
| `A` | Answer，应答来话 | `ATA` |
| `H` | Hang up，挂机 | `ATH`、`ATH0` |
| `O` | return to Online data state，回到数据态 | `ATO` |
| `E` | Echo，命令回显 | `ATE0` 关闭回显、`ATE1` 开启 |
| `T` / `P` | Tone / Pulse dialing，音频 / 脉冲拨号 | `ATDT` |
| `S` | 寄存器读写，格式 `ATS<n>=<v>` / `ATS<n>?` | `ATS0=1` |
| `+` | 扩展指令，由 3GPP 或厂商定义 | `AT+CSQ`、`AT+XGAUTH=…` |

## 5. AT 指令助记符

下表含义取自 3GPP TS 27.007 的条款标题，用于理解指令命名：

| 指令 | 所在条款标题 |
|------|--------------|
| `+CGDCONT` | Define PDP Context，定义 PDP 上下文 |
| `+CGACT` | PDP context activate or deactivate，激活 / 去激活 |
| `+CREG` | Network registration status，网络注册状态 |
| `+CGREG` | GPRS network registration status，分组域注册状态 |
| `+CSQ` | Signal quality，信号质量 |
| `+CPIN` | PIN，SIM 口令与状态 |
| `+COPS` | Operator selection，运营商选择 |
| `+CMEE` | Report mobile equipment error，错误码上报格式 |
| `+CGMI` / `+CGMM` / `+CGMR` | Request manufacturer / model / revision identification |
| `+CME ERROR` / `+CMS ERROR` | 设备类 / 短信类错误结果码 |

---

# 参考链接

- `man pppd`（选项、脚本、退出码与 FILES 段）
- [pppd 手册（中文翻译）-CSDN](https://blog.csdn.net/ZHNEYU/article/details/125914444)
- [【网络协议详解】——PPP协议（学习笔记）-CSDN](https://blog.csdn.net/HinsCoder/article/details/130781224)
