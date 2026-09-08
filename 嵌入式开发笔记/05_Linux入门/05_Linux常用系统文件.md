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

# ——etc——

# /etc目录

`/etc` 是 Linux 系统中最核心的目录之一，名称源于 “Editable Text Configuration”（可编辑文本配置）的缩写，主要用于存放**系统和应用程序的配置文件**，是系统正常运行的 “配置中枢”。

## 1.核心特点

- **纯文本为主**
  绝大多数文件是人类可读的文本文件，方便直接编辑（如用 `vi`、`nano` 等工具）。
- **按功能分类**
  目录结构按服务、应用或功能划分，便于管理和查找。
- **系统级配置**
  修改`/etc`下的文件通常需要 root 权限，且错误配置可能导致系统或程序异常。

## 2.常用子目录与文件

### 1.系统基础配置

- `/etc/passwd`：用户账号信息（用户名、UID、家目录等）。
- `/etc/group`：用户组信息（组名、GID、成员列表）。
- `/etc/shadow`：用户密码哈希（加密存储，仅 root 可读写）。
- `/etc/fstab`：开机自动挂载的文件系统配置（如硬盘、U 盘的挂载规则）。
- `/etc/hostname`：系统主机名。
- `/etc/hosts`：本地域名解析表（优先于 DNS 生效）。

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

`/etc` 目录是 Linux 系统的 “配置大脑”，包含了从用户管理、网络设置到服务运行的所有核心规则。理解其结构和文件用途，是管理 Linux 系统的基础 —— 无论是修改网络参数、配置服务，还是自定义设备行为，都离不开对 `/etc` 下文件的操作。

如果需要查找某个具体配置，可通过 `grep` 或 `find` 命令搜索（如 `grep -r "关键词" /etc/`）。

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

## 4. 系统级` crontab`

除用户` crontab `外，系统级任务可放在以下目录：

- `/etc/crontab`：系统全局` crontab `文件
- `/etc/cron.d/`：存放独立的 `crontab `文件
- `/etc/cron.hourly/`、`/etc/cron.daily/` 等：存放按时间自动执行的脚本

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