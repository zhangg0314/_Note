# ——proc目录——

# CPU信息

## 1.查看CPU硬件信息

```shell
cat /proc/cpuinfo

#/proc/cpuinfo 包含 CPU 硬件信息，其中 model name、Hardware 或 Processor 字段会显示 CPU 型号或厂商相关信息：
```

示例：9G25平台集中器

```shell
[root@/home/app]$cat /proc/cpuinfo
processor	: 0
model name	: ARM926EJ-S rev 5 (v5l)
BogoMIPS	: 199.06
Features	: swp half thumb fastmult edsp java 
CPU implementer	: 0x41
CPU architecture: 5TEJ
CPU variant	: 0x0
CPU part	: 0x926
CPU revision	: 5

Hardware	: Atmel AT91SAM9  #SOC平台
Serial		: 0000000000000000
```

## 2.查看芯片SOC具体型号

```shell
dmesg | grep AT91SAM9 #SOC平台

#示例
[root@/home/app]$dmesg | grep AT91SAM9
Machine model: Atmel AT91SAM9G25-EK
```

# ——etc目录——

# 目录介绍

`/etc` 是 Linux 系统中最核心的目录之一，名称源于 Editable Text Configuration（可编辑文本配置）的缩写，主要用于存放**系统和应用程序的配置文件**，是系统正常运行的配置中枢。其包含了从用户管理、网络设置到服务运行的所有核心规则。理解其结构和文件用途，是管理Linux系统的基础。无论是修改网络参数、配置服务，还是自定义设备行为，都离不开对 `/etc` 下文件的操作。如果需要查找某个具体配置，可通过 `grep` 或 `find` 命令搜索（如 `grep -r "关键词" /etc/`）。

## 1.核心特点

- **纯文本为主**
  绝大多数文件是人类可读的文本文件，方便直接编辑（如用 `vi`、`nano` 等工具）。
- **按功能分类**
  目录结构按服务、应用或功能划分，便于管理和查找。
- **系统级配置**
  修改`/etc`下的文件通常需要 root 权限，且错误配置可能导致系统或程序异常。

## 2.配置文件命名

`/etc` 下常见「同名文件与同名目录并存」的写法，例如 `rsyslog.conf` 与 `rsyslog.d/`。这并非分类混乱，而是刻意分工。事实上 `/etc` 并非按抽象的功能树划分，而是按**软件包 / 守护进程名**划分：安装了 rsyslog 便出现 `rsyslog.conf` 与 `rsyslog.d/`，安装了 nginx 便出现 `nginx/`，各软件包占用各自的命名空间。

### 1.`.conf`主配置文件

- 由**软件包**提供并维护，属于软件包的文件清单。以 Debian/Ubuntu 为例，可用 `dpkg -S /etc/rsyslog.conf` 查询归属，结果为 `rsyslog` 包（RedHat/CentOS 对应 `rpm -qf /etc/rsyslog.conf`）。
- 存放该服务的默认 / 基础配置。
- 升级软件包（`apt upgrade rsyslog` 或全局 `apt upgrade`）时，包管理器会重新解包新版本并覆盖该文件。若该文件曾被手工修改，Debian/Ubuntu 会将其识别为 conffile，升级时提示选择「采用维护者版本」或「保留本地版本」。

### 2.`.d`drop-in 扩展目录

- `.d` 目录由软件包创建（通常为空），但用户或其他软件包放入其中的文件**不在**该包的文件清单内。
- 升级软件包时不会触碰这些文件，因此自定义配置不会丢失。
- 作用
  向主配置追加或覆盖规则，而无须修改主文件。

### 3.加载机制

主配置文件末尾通过 include 引入 `.d` 目录下的全部片段。以 rsyslog 为例：

```conf
$IncludeConfig /etc/rsyslog.d/*.conf
```

服务启动时先读取 `.conf` 主配置（作为默认值），再按文件名顺序加载 `.d` 下的片段，后加载的配置可覆盖先加载的默认值。

1. **加载顺序**
   主文件须先加载（默认值），`.d` 片段后加载（覆盖值）。混入同一目录则顺序无保证。
2. **所有权**
   主文件归软件包维护者，`.d` 内文件归各软件包或管理员。混放后包升级无法区分哪些可覆盖、哪些须保留。
3. **可扩展性**
   `.d` 支持一对多，即一个主配置对应多个可插拔片段，增删片段即可增删规则，避免反复修改单个长文件。

### 4.同类约定

该约定在系统中通用：

- `/etc/apt/sources.list` 与 `/etc/apt/sources.list.d/`
- `/etc/profile` 与 `/etc/profile.d/`
- `/etc/cron.d/`、`/etc/cron.daily/`、`/etc/cron.hourly/`
- `/etc/modprobe.d/`、`/etc/logrotate.d/`、`/etc/udev/rules.d/`

识别规则：`xxx.conf` 为单文件、软件包默认配置；`xxx.d/` 为多文件、可扩展、供追加片段。

## 3.常用文件

### 1.系统基础配置

- `/etc/passwd`
  用户账号信息（用户名、UID、家目录等）。
- `/etc/group`
  用户组信息（组名、GID、成员列表）。
- `/etc/shadow`
  用户密码哈希（加密存储，仅 root 可读写）。
- `/etc/fstab`
  开机自动挂载的文件系统配置（如硬盘、U 盘的挂载规则）。
- `/etc/hostname`
  系统主机名。
- `/etc/hosts`
  本地域名解析表（优先于 DNS 生效）。

### 2.网络配置

- `/etc/network/interfaces`（Debian/Ubuntu）或 `/etc/sysconfig/network-scripts/`（RedHat/CentOS）：网络接口（如 eth0、wlan0）的 IP、网关等配置。
- `/etc/resolv.conf`：DNS 服务器地址配置。
- `/etc/services`：系统服务与端口号的对应关系（如 SSH 对应 22 端口）。

### 3.服务与进程管理

- `/etc/systemd/`：systemd 服务的配置文件（`.service`），用于管理系统服务的启动 / 停止（如你之前提到的 `usb-mount@.service` 可能放在这里）。
- `/etc/init.d/`：传统 SysV 风格的服务启动脚本（如 `ssh`、`nginx` 的启动脚本）。
- `/etc/cron.d/`、`/etc/crontab`：定时任务（cron）的配置文件。

### 4.应用程序配置

- `/etc/ssh/`：SSH 服务的配置（如 `sshd_config` 控制 SSH 服务器行为）。
- `/etc/nginx/`、`/etc/apache2/`：Web 服务器（Nginx/Apache）的配置目录。
- `/etc/mysql/`、`/etc/postgresql/`：数据库服务的配置目录。
- `/etc/sudoers`：`sudo` 命令的权限配置（控制哪些用户可执行 root 命令）。

### 5.其他重要文件

- `/etc/profile`、`/etc/bashrc`：系统级的 shell 环境变量配置（对所有用户生效）。
- `/etc/issue`、`/etc/motd`：登录前显示的提示信息（如欢迎语）。
- `/etc/udev/rules.d/`：udev 设备管理规则（如你之前的 USB 自动挂载规则放在这里）。

### 6.总结



- **终端环境配置文件**
  当登录系统或新开启一个ssh连接启动bash进程时，一定会加载这4个配置文件：

  ```bash
  vim /etc/profile   #系统全局环境和登录系统的一些配置
  
  vim /etc/bashrc    #shell全局自义配置文件，用于自定义所有shell
  
  vim /user/.bashrc  #用于单独自定义某个用户的bash
  
  [root@zhangg ~]# vim /root/.bash_profile  #用户单独自定义某个用户的系统环境
  ```

- **网络配置文件**
  `/etc/network/interfaces` 是一个传统的网络配置文件，适用于 Debian 及其衍生发行版。虽然在现代系统中，其功能已经被 Netplan 或 NetworkManager 所取代，但了解它的使用方法仍然很有价值，尤其是在需要手动配置网络接口时。`ifconfig` 命令用于临时配置网络接口的参数，这些配置仅在当前会话中有效。一旦系统重启，这些临时配置会被丢弃，系统会根据网络配置文件重新初始化网络接口。`/etc/network/interfaces` 文件是系统启动时用于配置网络接口的配置文件。在系统启动过程中，网络服务会读取该文件中的配置信息，并根据这些信息设置网络接口的参数。因此，通过 `/etc/network/interfaces` 文件配置的静态 IP 地址会在每次重启后仍然有效。

  ```vim
  # 全局配置
  auto lo
  iface lo inet loopback
  
  # 网络接口配置
  auto eth0
  iface eth0 inet static
      address 192.168.1.100
      netmask 255.255.255.0
      gateway 192.168.1.1
      dns-nameservers 8.8.8.8 8.8.4.4
  
  auto wlan0
  iface wlan0 inet dhcp
      wpa-ssid "your_wifi_ssid"
      wpa-psk "your_wifi_password"
  ```

### 2./dev目录下的文件

- **空设备文件**
  在Linux系统中，空设备文件`/dev/null`是一个特殊的文件，用于将数据彻底丢弃。它是一个特殊的文件设备，可以通过标准输入和标准输出访问。读取/dev/null将会立即返回EOF文件结束符，而向它写入任何数据将会直接被忽略掉，从而丢弃数据。Linux中，默认命令、脚本等的标准正常输出和标准错误输出默认是输出到标准终端上，即：屏幕。如果不想在标准终端上显示，可把结果输出到/dev/null中。

## 4.文件系统挂载表（fstab）

`/etc/fstab` 是文件系统挂载表（FileSystem TABle），定义系统**开机时自动挂载**的文件系统。系统启动时，init 进程读取该表，按规则将各分区挂载到指定挂载点。

### 1.文件格式

每行一条挂载规则，共 6 个字段，以空格 / Tab 分隔：

```
<设备>  <挂载点>  <文件系统类型>  <挂载选项>  <dump>  <fsck 顺序>
```

示例：

```bash
/dev/sda1   /mnt/data   ext4   defaults   0   2
UUID=xxxx   /boot       vfat   rw         0   0
```

| 字段 | 含义 | 示例 |
|------|------|------|
| 设备 | 待挂载设备 | `/dev/sda1`、`UUID=xxxx`、`LABEL=xxxx` |
| 挂载点 | 挂载目标目录 | `/mnt/data`、`/boot` |
| 文件系统类型 | 分区格式 | `ext4`、`vfat`、`ntfs`、`auto` |
| 挂载选项 | 挂载行为 | `defaults`、`noatime`、`ro`、`rw` |
| dump | 备份标志 | `0`（不备份，基本废弃） |
| fsck 顺序 | 开机检查顺序 | `0`=不检查，`1`=根分区，`2`=其他 |

### 2.嵌入式设备中的 fstab

嵌入式设备（如本平台的 AT91SAM9）的 fstab 可能为空，仅含占位注释：

```bash
# UNCONFIGURED FSTAB FOR BASE SYSTEM
```

原因：嵌入式设备根文件系统**不依赖 fstab 挂载**，而是通过：

1. **内核启动参数 `root=`**：由内核在启动时直接挂载根分区（如 `root=/dev/mtdblock0`、`root=ubi0:rootfs`）。
2. **initramfs**：在早期用户空间先挂载根文件系统，再切换到真实根。

fstab 仅负责「额外」的挂载（数据分区、tmpfs、U 盘、交换分区等），无此需求时空表即可。

### 3.查看实际挂载情况

fstab 只描述「开机要挂什么」，不代表当前实际挂载状态。查看真实挂载：

```bash
mount              # 当前所有挂载点（实际生效）
cat /proc/mounts   # 内核视角的挂载表
cat /proc/cmdline  # 内核启动参数（查看 root= 如何指定）
```

嵌入式设备根分区常为 NAND flash（`/dev/mtdblock0`）或 UBI（`ubi0:rootfs`），与 x86 桌面机的 `/dev/sda1` 不同。

## 5.Shell 配置文件（/etc/profile 与 profile.d）

Shell 配置文件按「登录 shell」与「交互式 shell」两个维度区分加载时机。登录 shell（ssh 登录、控制台登录、`su -`）启动时，依次读取系统级与用户级配置。

### 1.`/etc/profile` 解析

`/etc/profile` 是所有 Bourne 系 shell（sh/bash/ksh/ash）共享的系统级登录配置。Debian/Ubuntu 默认内容如下：

```bash
# /etc/profile: system-wide .profile file for the Bourne shell (sh(1))
# and Bourne compatible shells (bash(1), ksh(1), ash(1), ...).

if [ "${PS1-}" ]; then
  if [ "${BASH-}" ] && [ "$BASH" != "/bin/sh" ]; then
    if [ -f /etc/bash.bashrc ]; then
      . /etc/bash.bashrc
    fi
  else
    if [ "`id -u`" -eq 0 ]; then
      PS1='# '
    else
      PS1='$ '
    fi
  fi
fi

if [ -d /etc/profile.d ]; then
  for i in /etc/profile.d/*.sh; do
    if [ -r $i ]; then
      . $i
    fi
  done
  unset i
fi
```

逐块说明：

- **`if [ "${PS1-}" ]`**：判断是否为交互式 shell。交互式 shell 存在 `PS1`（主提示符），非交互式（执行脚本）没有。`${PS1-}` 中 `-` 表示变量未定义时展开为空，避免报错。
- **`[ "${BASH-}" ] && [ "$BASH" != "/bin/sh" ]`**：判断是否为「真 bash」。`$BASH` 为 bash 可执行路径，经 `/bin/sh` 调用时 `$BASH=/bin/sh`，被排除。
- **真 bash** → `source /etc/bash.bashrc`（bash 全局 rc）。
- **非 bash**（纯 sh/ash/ksh）→ 手动设置提示符：`id -u` 返回当前 UID，`0` 为 root，root 提示符 `# `，普通用户 `$ `。

### 2.profile.d 加载机制

`/etc/profile.d/` 是 `/etc/profile` 的 drop-in 扩展目录。`/etc/profile` 末尾通过循环将目录下所有 `*.sh` 脚本 source 进来：

```bash
for i in /etc/profile.d/*.sh; do
  if [ -r $i ]; then
    . $i
  fi
done
```

- 数字前缀（`01-`、`02-`）控制加载顺序。
- 脚本可读（`-r`）才加载，`. $i` 表示在当前 shell 执行（非子进程）。
- 软件包 / 管理员无需修改 `/etc/profile` 主文件，投放脚本到该目录即可为所有登录 shell 注入配置。

### 3.登录 shell 完整加载链

```
bash 登录
  → /etc/profile          （系统级登录配置）
      → /etc/bash.bashrc   （真 bash 才 source）
      → /etc/profile.d/*.sh（循环 source）
  → ~/.profile            （用户级登录配置）
      → ~/.bashrc
```

### 4.相关概念

| 概念 | 含义 |
|------|------|
| login shell | 登录时启动的 shell，读 `/etc/profile` → `~/.profile` |
| 交互式 shell | 存在 `PS1` 提示符 |
| `id -u` | 当前用户 UID，0 = root |
| `. file` / `source file` | 在当前 shell 执行，非子进程 |
| `PS1` | 主提示符，`#` = root，`$` = 普通用户 |

# 设备管理（udev）

## 1.基本概念

`udev` 是 Linux 用户空间中的设备管理器，以 `systemd-udevd` 守护进程形式运行。当内核检测到设备热拔插时，`udev` 在用户空间作出响应，完成以下任务：

- 在 `/dev` 目录下动态创建或删除设备节点（如 `/dev/sda1`、`/dev/ttyUSB0`）。
- 根据规则为设备命名、设置权限、建立符号链接。
- 加载相应驱动模块。
- 触发用户自定义动作（执行脚本、启动服务等）。

`udev` 是连接内核与用户空间的桥梁：内核负责发现硬件并上报事件（uevent），`udev` 负责在用户空间执行匹配的规则。

## 2.规则文件存放位置

`udev` 规则文件以 `.rules` 为后缀，按目录优先级加载：

| 路径 | 优先级 | 用途 |
|------|--------|------|
| `/etc/udev/rules.d/` | 最高 | 管理员自定义规则 |
| `/usr/lib/udev/rules.d/`（或 `/lib/udev/rules.d/`） | 较低 | 发行版 / 软件包自带规则 |

同名规则文件时，`/etc/udev/rules.d/` 下的优先。规则文件按文件名字母顺序加载，故用数字前缀（`10-`、`50-`、`80-`）控制加载顺序。

## 3.规则语法

规则由「匹配键」与「动作键」组成，基本格式：

```
匹配键==值, 匹配键==值, ..., 动作键=值
```

- 同一规则内的多个匹配键为 **AND** 关系（全部满足才执行动作）。
- 多条规则之间为 **OR** 关系（任一规则命中即执行）。
- 常用运算符：`==`（等于）、`!=`（不等于）、`+=`（追加）、`=`（赋值）、`:=`（最终赋值，不可被后续覆盖）。

### 1.常用匹配键

| 匹配键 | 含义 |
|--------|------|
| `ACTION` | 事件类型：`add`（插入）、`remove`（拔出）、`change`（属性变更） |
| `KERNEL` | 内核设备名（如 `sda1`、`ttyUSB0`） |
| `SUBSYSTEM` | 设备自身所属子系统（如 `block`、`usb`、`net`） |
| `SUBSYSTEMS` | 设备链路上所有父设备的子系统（复数，用于判断接口类型） |
| `ATTR{...}` | 设备 sysfs 属性（如 `ATTR{idVendor}`） |
| `ENV{...}` | 环境变量 / `udev` 属性 |

`SUBSYSTEM`（单数）表示设备本身的子系统（U 盘为 `block`）；`SUBSYSTEMS`（复数）向上遍历所有父设备，用于区分接口类型（如排除 SATA 硬盘、只匹配 USB 接口设备）。

### 2.常用动作键

| 动作键 | 含义 |
|--------|------|
| `RUN+=` | 执行命令 / 脚本 |
| `SYMLINK+=` | 建立符号链接 |
| `MODE=` | 设置设备节点权限 |
| `OWNER=` / `GROUP=` | 设置属主 / 属组 |
| `TAG+="systemd"` | 标记设备交由 systemd 管理 |
| `ENV{SYSTEMD_WANTS}=` | 指定要启动的 systemd 单元 |

### 3.替换符

- `%k`：内核设备名（`KERNEL` 的值，如 `sda1`）
- `%n`：内核设备序号（如 `1`）
- `%p`：设备路径（如 `/sys/block/sda/sda1`）
- `$env{...}`：引用环境变量

## 4.热拔插触发 systemd 服务的两种方式

`udev` 检测到热拔插后，可通过两种方式触发 systemd 服务：

### 方式一：直接调用 systemctl（老式写法）

```bash
ACTION=="add", KERNEL=="sd[a-z][0-9]*", SUBSYSTEMS=="usb", RUN+="/bin/systemctl start usb-mount@%k.service"
ACTION=="remove", KERNEL=="sd[a-z][0-9]*", SUBSYSTEMS=="usb", RUN+="/bin/systemctl stop usb-mount@%k.service"
```

`RUN` 中直接调用 `systemctl`，简单直观，但会阻塞 `udev` 事件处理线程。嵌入式与老系统中大量使用。

### 方式二：SYSTEMD_WANTS 标记（推荐写法）

```bash
ACTION=="add", KERNEL=="sd[a-z][0-9]*", SUBSYSTEMS=="usb", TAG+="systemd", ENV{SYSTEMD_WANTS}="usb-mount@%k.service"
```

通过 `TAG+="systemd"` 与 `ENV{SYSTEMD_WANTS}` 交由 systemd 异步启动服务，不阻塞 `udev`，为官方推荐方式。

## 5.完整实例：USB 自动挂载升级

### 1.udev 规则

```bash
ACTION=="add",KERNEL=="sd[a-z][0-9]*",SUBSYSTEMS=="usb",RUN+="/bin/systemctl start usb-mount@%k.service"
ACTION=="remove",KERNEL=="sd[a-z][0-9]*",SUBSYSTEMS=="usb",RUN+="/bin/systemctl stop usb-mount@%k.service"
```

`%k` 展开为内核设备名（如 `sda1`），实例化出 `usb-mount@sda1.service`。

### 2.调用链路

```
U 盘插入 → 内核上报 uevent → udev 规则命中 → systemctl start usb-mount@sda1.service
        → systemd 读取模板 usb-mount@.service → ExecStart 执行脚本，脚本参数 $1=/dev/sda1
```

`%k`（udev 侧）与 `%i`（service 模板侧）为同一值，最终以 `/dev/%i` 形式作为脚本参数传入。

## 6.常用命令

```bash
udevadm control --reload-rules   # 重新加载规则（修改 .rules 后执行）
udevadm trigger                   # 重新触发已有设备事件（使新规则生效）
udevadm monitor                   # 实时监控设备事件（调试用）
udevadm info -a -n /dev/sda1      # 查看设备属性（匹配键取值来源）
```

`udevadm monitor` 配合插入 / 拔出设备，可观察内核事件与规则匹配过程，是排查热拔插问题的主要手段。

# 服务自启动（SysV init / 运行级别）

Linux 服务管理历经三代 init 系统，配置目录随代际更替：

| 时代 | init 系统 | 配置目录 | 文件形式 | 状态 |
|------|-----------|---------|---------|------|
| 传统 | SysV init | `/etc/init.d/`、`/etc/rcN.d/` | shell 脚本 | 被取代，兼容保留 |
| 过渡 | Upstart | `/etc/init/` | `.conf` job | 已废弃 |
| 现代 | systemd | `/etc/systemd/` | `.service` 单元 | 当前主流 |

本章介绍 SysV init 的运行级别机制，作为理解 systemd 兼容层的基础。

## 1.`/etc/init.d/` 脚本

SysV 服务脚本存放在 `/etc/init.d/`，本质是**带参数的 shell 脚本**，支持 `start` / `stop` / `restart` 等参数：

```bash
/etc/init.d/nginx start    # 启动
/etc/init.d/nginx stop     # 停止
/etc/init.d/nginx restart  # 重启
```

脚本存在并不等于开机自启，开机自启需依赖运行级别目录中的软链接。

## 2.运行级别（runlevel）

运行级别是 SysV init 定义的系统状态，共 7 个（0~6）：

| 级别 | 含义 | 说明 |
|------|------|------|
| 0 | 关机（halt） | 关闭系统 |
| 1 | 单用户模式 | 仅 root、最小环境，维护 / 救援用 |
| 2 | 多用户（图形） | Ubuntu 默认级别 |
| 3 | 同级别 2 | 未使用 |
| 4 | 同级别 2 | 未使用 |
| 5 | 同级别 2 | 未使用 |
| 6 | 重启（reboot） | 重新启动 |

Ubuntu（Debian 系）运行级别特点：2~5 完全等价，均为多用户模式，默认进入级别 2。图形界面还是文本界面**不取决于运行级别**，而取决于是否安装了显示管理器（`gdm` / `lightdm`）。RedHat 系才严格区分：3 为文本多用户、5 为图形多用户。

划分多个级别的原因：系统需进入不同状态、运行不同的服务集合。正常使用需完整服务；故障排查需单用户最小环境；关机 / 重启为特殊状态。

## 3.`/etc/rcN.d/` 与 S/K 软链接

`/etc/rcN.d/`（N 为运行级别）存放指向 `/etc/init.d/` 的软链接：

- **`S` 开头**：进入该级别时执行（`脚本 start`）
- **`K` 开头**：离开该级别时执行（`脚本 stop`）
- **数字**：执行顺序，越小越先执行

示例：

```bash
/etc/rc2.d/S01hostname.sh  ->  /etc/init.d/hostname.sh
/etc/rc2.d/K20dbdaily.sh   ->  /etc/init.d/dbdaily.sh
```

启动顺序决定服务依赖关系：基础服务（网络、数据库）数字小、先启动；业务应用数字大、最后启动（如 `S99runapplications`），保证依赖就绪。

### 启动顺序号

- 取值范围：两位数字 `00` ~ `99`，前导零补齐（`01`、`09`、`99`）。
- 执行方向：`S` 与 `K` 均按数字升序执行。`S` 数字小先启动、大后启动；`K` 数字小先停止、大后停止。
- **字典序陷阱**：软链接按 ASCII 字典序排序而非数值排序，故必须固定两位。`S100` 在字典序中排 `S99` 之前（`"1" < "9"`），会导致顺序错乱。

数字分段惯例：

| 数字段 | 典型用途 |
|--------|---------|
| 00~19 | 系统底层（hostname、网络、文件系统） |
| 20~39 | 基础服务（数据库、日志） |
| 40~59 | 网络服务（sshd、Web） |
| 60~89 | 应用服务 |
| 90~99 | 业务应用（最后启动，等依赖就绪） |

### Ubuntu 上为何放级别 3

Ubuntu（Debian 系）2~5 等价，放级别 2 还是 3 无功能差异。脚本放 `rc3.d` 多沿用 RedHat 的「3 = 服务器标准级别」习惯，属历史惯性而非必要。

但须注意：软链接所在级别须与系统实际进入的级别匹配。Ubuntu 默认进入级别 2，若仅在 `rc3.d` 放置 `S` 软链接，则系统进入级别 2 时不会执行。正确做法是使用 `update-rc.d xxx defaults`，在 `rc2.d` ~ `rc5.d` 均创建 `S` 软链接，覆盖所有多用户级别。

## 4.启用自启 = 创建软链接

脚本放入 `/etc/init.d/` 后不会自动自启，须显式创建软链接：

```bash
# Debian/Ubuntu
update-rc.d nginx defaults   # 创建各级别 S/K 软链接 = 启用
update-rc.d nginx remove     # 删除软链接 = 禁用

# RedHat/CentOS
chkconfig nginx on           # 启用
chkconfig nginx off          # 禁用
```

## 5.切换运行级别

```bash
init 0           # 关机
init 6           # 重启
init 1           # 进入单用户模式
telinit 3        # 切换到级别 3
runlevel         # 查看当前级别
```

## 6.systemd 的兼容与对应

systemd 用 target 取代 runlevel，保留映射：

| runlevel | systemd target |
|----------|----------------|
| 0 | `poweroff.target` |
| 1 | `rescue.target` |
| 3 | `multi-user.target` |
| 5 | `graphical.target` |
| 6 | `reboot.target` |

systemd 启动时，`systemd-sysv-generator` 扫描 `/etc/init.d/` 与 `/etc/rcN.d/`，将 SysV 脚本自动转换为 `.service` 单元（标记 `generated`），并依据 `S` 软链接决定是否开机自启。因此系统虽已迁移至 systemd，`/etc/init.d/` 与 `/etc/rcN.d/` 仍作为兼容层保留。

# 服务自启动（systemd）

## 1.`systemd`服务

`systemd`是 Linux 系统中广泛使用的系统和服务管理器，它取代了传统的`SysV`，负责系统启动、服务管理、进程监控等核心功能。其设计目标是提高系统启动速度、优化服务依赖管理，并提供更丰富的系统管理能力。适合长期稳定运行的命令，通过系统服务管理器 `systemd` 管理，支持开机自启、日志保存、状态监控等。

## 2.核心功能与特点

1. **并行启动服务**
   传统 `SysVinit` 按顺序启动服务，而` systemd` 能并行启动无依赖关系的服务，大幅缩短系统启动时间。
2. **统一的服务管理接口**
   通过 `systemctl` 命令统一管理系统服务（启动、停止、重启、查看状态等），替代了传统的 `/etc/init.d/` 脚本。
3. **基于单元（Unit）的管理**
   `systemd` 用 “单元” 描述系统资源，包括服务（.service）、挂载点（.mount）、设备（.device）等，每种单元有特定的配置文件（通常位于 `/usr/lib/systemd/system/` 或 `/etc/systemd/system/`）。
4. **自动重启与监控**
   可配置服务崩溃时自动重启，通过 `systemctl status` 实时查看服务状态和日志。
5. **依赖管理**
   单元配置中可明确依赖关系（如 `After=network.target` 表示服务在网络启动后运行），<u>避免手动处理启动顺序</u>。

## 3.常用`systemctl`命令

| 命令                                       | 功能描述                                            |
| ------------------------------------------ | --------------------------------------------------- |
| `systemctl start <服务名>`                 | 启动指定服务                                        |
| `systemctl stop <服务名>`                  | 停止指定服务                                        |
| `systemctl restart <服务名>`               | 重启指定服务                                        |
| `systemctl enable <服务名>`                | **设置服务开机自启动**                              |
| `systemctl disable <服务名>`               | 取消服务开机自启动                                  |
| `systemctl status <服务名>`                | 查看服务状态（运行 / 停止等）                       |
| `systemctl list-unit-files --type=service` | 列出所有服务及自启动状态                            |
| `systemctl daemon-reload**`**              | **重新加载 `systemd` 配置（修改服务文件后需执行）** |

## 4.服务配置文件

在 Linux 系统中，`.server` 文件通常用于定义 **`systemd` 服务单元**，是管理系统服务（如后台进程、守护程序）的配置文件。

### 1.文件存储路径

文件名通常为 `服务名.service`（例如 `nginx.service`、`myapp.service`），存放路径一般为：

- 系统级服务：`/usr/lib/systemd/system/` 或 `/etc/systemd/system/`
- 用户级服务：`~/.config/systemd/user/`

### 2.基本结构

```ini
[Unit]          # 服务的元数据（描述、依赖关系等）
[Service]       # 服务的核心配置（启动命令、运行方式等）
[Install]       # 服务的安装配置（开机启动相关）
```

举例：

```ini
[Unit]# 服务的元数据（描述、依赖关系等）
Description=My Application Service  # 服务描述
After=network.target                # 依赖网络服务启动后运行

[Service]# 服务的核心配置（启动命令、运行方式等
Type=simple                         # 服务类型（simple/forking等）
ExecStart=/usr/bin/myapp            # 启动命令
Restart=on-failure                  # 失败时自动重启
User=nobody                         # 运行用户

[Install]# 服务的安装配置（开机启动相关）
WantedBy=multi-user.target          # 开机自启动时的目标级别
```

将文件放入 `/etc/systemd/system/` 后，执行 `systemctl daemon-reload` 即可通过 `systemctl` 管理该服务。

### 3.模板单元与实例化（`@` 与 `%i`）

文件名含 `@` 的服务为**模板单元**，如 `usb-mount@.service`、`autovt@.service`。模板本身不直接运行，而是按需实例化出具体实例，如 `usb-mount@sda1.service`。

- `%i`：实例名，即 `@` 与 `.service` 之间的部分（如 `sda1`），可在模板内部引用。
- 同一模板可并行启动多个实例（如同时插入多个 U 盘，对应 `usb-mount@sda1`、`usb-mount@sdb1`）。

实例化来源通常为 `udev` 热拔插事件（详见「设备管理（udev）」章节），`udev` 规则中的 `%k` 传入后作为 `%i` 使用。

示意（实际配置依设备而定）：

```ini
# /etc/systemd/system/usb-mount@.service
[Service]
Type=oneshot
ExecStart=/usr/bin/automount.sh /dev/%i
ExecStop=/bin/umount /mnt/usbflash
```

`ExecStart` 中的 `/dev/%i` 展开为 `/dev/sda1`，作为脚本的 `$1` 参数传入，完成从 `udev` 到脚本的参数传递。

## 5.日志管理

使用 `journalctl` 查看系统日志：

```bash
journalctl -u xxx.service -f#动态打印日志
```

## 6.服务状态

在 systemd 中，服务（`.service`）的 `STATE`（状态）反映了服务的启用状态、运行状态或配置属性，不同状态有明确的含义。以下是一些服务状态解析：

### 1.`enabled`（已启用）

- **含义**：服务被配置为 “开机自动启动”（随系统启动而启动），且当前可能处于运行中（`active`）或未运行（`inactive`）状态，仅表示 “自动启动开关已打开”。
- **示例**：
  - `app-start.service`：开机时会自动启动该应用服务。
  - `console-setup.service`：系统启动时自动配置控制台（如字体、编码）。
  - `cron.service`：定时任务服务，开机自启以保证定时任务生效。
  - `autovt@.service`：自动启动虚拟终端（如 tty1），方便用户登录。

### 2.`disabled`（已禁用）

- **含义**：服务被配置为 “开机不自动启动”，但可以通过 `systemctl start` 手动启动。
- **示例**：
  - `console-getty.service`：默认不自动启动额外的控制台登录服务（仅保留必要的终端）。

### 3.`static`（静态）

- **含义**：服务本身没有 “自动启动” 配置（无 `[Install]` 段定义启动链接），无法通过 `systemctl enable` 启用自动启动，只能被其他服务 “按需触发”（**作为依赖被拉起**）。
- **示例**：
  - `apt-daily.service`/`apt-daily-upgrade.service`：APT 自动更新服务，由系统定时器（`timer`）触发，而非直接开机自启。
  - `container-getty@.service`：容器的终端服务，仅当容器启动时被自动调用，无需独立开机自启。

### 4.`generated`（生成的）

- **含义**：服务文件不是手动编写的，而是由系统工具（如 `systemctl generate`）动态生成的临时配置，通常用于临时适配或自动兼容旧系统。
- **示例**：
  - `alsa-utils.service`：音频工具服务，可能由系统根据硬件自动生成配置。
  - `config.service`：可能是某个程序运行时动态生成的配置服务。
  - `cron_log.service`/`daydata2csv.service`：可能是日志或数据处理相关的动态生成服务。

### 5.`masked`（已屏蔽）

- **含义**：服务被 “彻底禁用”，不仅无法开机自启，甚至不能手动启动（`systemctl start` 会失败），相当于 “强制冻结”。通常用于禁用冗余或冲突的旧服务（如 SysV 时代的服务，在 systemd 系统中已被替代）。
- **示例**：
  - `bootlogd.service`/`bootlogs.service`：传统的启动日志服务，在 systemd 中已被 `journald` 替代，故屏蔽。
  - `checkfs.service`/`checkroot.service`：传统的文件系统检查服务，systemd 已通过 `systemd-fsck` 实现，故屏蔽。
  - `cryptdisks.service`：传统的加密磁盘服务，已被 `systemd-cryptsetup` 替代，故屏蔽。

### 6.状态核心区别

| 状态        | 自动启动 | 手动启动 | 典型用途                  |
| ----------- | -------- | -------- | ------------------------- |
| `enabled`   | 是       | 是       | 核心服务（如 cron、网络） |
| `disabled`  | 否       | 是       | 偶尔手动运行的服务        |
| `static`    | 否       | 间接触发 | 被其他服务依赖的辅助服务  |
| `generated` | 按需     | 是       | 动态生成的临时 / 兼容服务 |
| `masked`    | 否       | 否       | 已废弃、需彻底禁用的服务  |

可以通过 `systemctl status 服务名` 查看服务的详细状态（如是否正在运行），例如：

`systemctl status cron.service` 会显示 `active (running)` 或 `inactive (dead)` 等实时状态。

## 7.开机监控网口抓包流程

### 1.创建服务文件

创建一个系统服务配置文件，用于定义开机执行的命令和输出保存路径：

```bash
sudo vi /etc/systemd/system/tshark-capture.service
#粘贴以下内容

Description=Auto-run tshark capture on startup (Ubuntu)
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
ExecStart=/bin/bash -c 'stdbuf -oL /usr/bin/tshark -i eth0 -Y "(tcp.dstport == 9995 and ip.dst == 192.168.8.164) or (tcp.srcport == 9995 and ip.src == 192.168.8.164) and tcp.len > 5 " -T fields  -e frame.time  -e data -E separator=, -E occurrence=f >> /var/log/tshark_capture.log 2>&1'
Restart=always
User=root
Group=root
KillMode=control-group

[Install]
WantedBy=multi-user.target
```

核心参数说明：

- `>> /var/log/tshark_capture.log 2>&1`：将输出**追加**到日志文件（避免覆盖历史记录），同时记录错误信息。
- `Restart=always`：确保抓包命令持续运行（若意外中断会自动重启）。
- `User=root`：`tshark` 抓包需要 root 权限，必须指定 root 用户。

### 2.设置服务并启用开机自启

```bash
# 设置服务文件权限
sudo chmod 644 /etc/systemd/system/tshark-capture.service

# 重新加载systemd配置
sudo systemctl daemon-reload

# 启用开机自启
sudo systemctl enable tshark-capture.service

#启动服务
sudo systemctl start tshark-capture.service

# 查看服务状态（确认是否运行中）
sudo systemctl status tshark-capture.service
```

# 定时任务

## 1. 基本概念

- **`crontab` 文件**：存储定时任务的配置文件，每个用户有自己的 `crontab `文件。
- **`cron` 服务**：系统后台守护进程，负责读取 `crontab` 文件并按计划执行任务。

## 2. 常用命令

```bash
# 编辑当前用户的 crontab 文件
crontab -e

# 查看当前用户的 crontab 文件
crontab -l

# 删除当前用户的所有 crontab 任务
crontab -r

# 查看指定用户的 crontab（需 root 权限）
crontab -u username -l
```

## 3. crontab 文件格式

### 1.格式

```plaintext
* * * * * command
│ │ │ │ │
│ │ │ │ └── 星期 (0-7, 0和7都代表星期日)
│ │ │ └──── 月份 (1-12)
│ │ └────── 日期 (1-31)
│ └──────── 小时 (0-23)
└────────── 分钟 (0-59)
```

### 2.特殊符号

- `*`：任意值（匹配所有可能的值）
- `,`：分隔多个值（如 `1,3,5`）
- `-`：定义范围（如 `1-5`）
- `/`：定义间隔（如 `*/2` 表示每 2 个单位）

### 3. 示例任务

```bash
# 每天凌晨2点执行备份脚本
0 2 * * * /usr/local/bin/backup.sh

# 每小时的第15分钟执行日志清理
15 * * * * /var/log/cleanup.py

# 每周日早上8点重启服务
0 8 * * 0 service nginx restart

# 每月1号和15号执行系统更新
0 4 1,15 * * apt-get update && apt-get upgrade -y

# 每5分钟检查一次系统状态
*/5 * * * * /scripts/check_status.sh
```

### 4. 注意事项

1. **环境变量**：`crontab` 执行时的环境变量与登录 shell 不同，建议在脚本中明确指定路径。

   ```bash
   # 错误：可能找不到命令
   0 0 * * * backup.sh
   
   # 正确：指定完整路径
   0 0 * * * /usr/local/bin/backup.sh
   ```

2. **日志输出**：任务输出默认发送到用户邮箱，建议重定向到日志文件。

   ```bash
   # 将输出重定向到日志
   0 2 * * * /script/run.sh > /var/log/cronjob.log 2>&1
   ```

3. **执行顺序**：多个任务可能并行执行，若有依赖关系需注意。

## 4. 系统级 crontab

除用户 crontab 外，系统级定时任务由以下位置承载：

- `/etc/crontab`：系统全局 crontab 主文件
- `/etc/cron.d/`：存放独立的 crontab 片段文件（各软件包自行投放）
- `/etc/cron.hourly/`、`/etc/cron.daily/`、`/etc/cron.weekly/`、`/etc/cron.monthly/`：存放按固定间隔执行的脚本

### 1.系统 crontab 与用户 crontab 的区别

| 项目 | 用户 crontab | 系统 crontab（`/etc/crontab`、`/etc/cron.d/*`） |
|------|-------------|-----------------------------------------------|
| 字段数 | 5 字段 | 6 字段（多 `user` 字段） |
| 格式 | `m h dom mon dow command` | `m h dom mon dow user command` |
| 编辑方式 | `crontab -e`，需用 `crontab` 命令安装 | 直接编辑文件，无需安装，cron 启动时读取 |
| 运行身份 | 所属用户 | `user` 字段指定用户 |
| 环境变量 | 无 `SHELL`/`PATH` 字段 | 文件内可设 `SHELL`、`PATH` |

`user` 字段为系统 crontab 专属：指定任务以哪个用户身份运行。用户 crontab 无该字段，默认以该用户身份运行。

### 2.`/etc/crontab` 解析

```bash
SHELL=/bin/sh
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin

# m h dom mon dow user    command
17 *    * * *    root    cd / && run-parts --report /etc/cron.hourly
25 6    * * *    root    test -x /usr/sbin/anacron || ( cd / && run-parts --report /etc/cron.daily )
47 6    * * 7    root    test -x /usr/sbin/anacron || ( cd / && run-parts --report /etc/cron.weekly )
52 6    1 * *    root    test -x /usr/sbin/anacron || ( cd / && run-parts --report /etc/cron.monthly )
```

| 调度表达式 | 触发时间 | 动作 |
|-----------|---------|------|
| `17 * * * *` | 每小时第 17 分 | 执行 `cron.hourly/` 下所有脚本 |
| `25 6 * * *` | 每天 6:25 | 执行 `cron.daily/` 下所有脚本 |
| `47 6 * * 7` | 每周日 6:47 | 执行 `cron.weekly/` 下所有脚本 |
| `52 6 1 * *` | 每月 1 号 6:52 | 执行 `cron.monthly/` 下所有脚本 |

关键点：

- **`run-parts --report`**：按文件名顺序执行指定目录下所有可执行脚本，`--report` 表示执行后打印结果。
- **`test -x /usr/sbin/anacron || (...)`**：短路判断。若已安装 `anacron`，交由 anacron 处理（可补跑因关机而错过的任务），cron 不再执行；若未安装，才由 cron 直接执行 `run-parts`。
- **分钟错开**：17/25/47/52 为刻意错开的随机值，避免大量机器在同一瞬间同时触发任务。

### 3.`cron.d/` 与 `cron.daily/` 的区别

| | `cron.d/` | `cron.hourly/` ~ `cron.monthly/` |
|--|-----------|----------------------------------|
| 存放内容 | crontab 格式文件（带时间字段） | 可执行脚本（无时间字段） |
| 时间控制 | 文件内自行指定 | 由 `/etc/crontab` 统一调度 |
| `user` 字段 | 有 | 无 |
| 适用场景 | 精确控制执行时间 | 粗粒度（每小时/每天一次） |

软件包需新增定时任务时：需精确时间 → 投放 crontab 文件至 `cron.d/`；仅需按固定周期执行 → 投放脚本至 `cron.daily/` 等目录。

## 5. 调试技巧

```bash
# 临时添加调试信息
* * * * * /path/to/script.sh >> /tmp/cron_debug.log 2>&1

# 检查 cron 服务状态
service cron status

# 查看系统日志（Ubuntu/Debian）
tail -f /var/log/syslog | grep cron

# 查看系统日志（CentOS/RHEL）
tail -f /var/log/cron
```

## 6. 在线生成工具

若觉得手动编写复杂，可使用在线生成器辅助：

- [crontab.guru](https://crontab.guru/)：输入描述自动生成 `cron` 表达式

合理使用 `cronab` 可以自动化日常运维任务，提高工作效率。