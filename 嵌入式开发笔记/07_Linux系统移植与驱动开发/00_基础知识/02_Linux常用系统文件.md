## 4.常用配置目录/文件

### 1./etc目录下的文件

`/etc` 目录是 Linux 系统中用于存放系统配置文件的标准目录。许多重要的配置文件都存放在 `/etc` 目录下，这些文件在系统启动时会被不同的服务和程序读取，以完成系统的初始化和配置。

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

# `crontable`

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