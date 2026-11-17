# ——SSH远程连接——

# `SSH`介绍及应用

## 1.概念定义

**SSH（Secure Shell，安全外壳协议）** 是一种建立在应用层和传输层基础上的网络协议，用于在不安全的网络环境中为两台计算机之间提供加密的远程登录、命令执行、文件传输等安全通信服务。SSH通过**数据加密**、**身份认证**和**完整性校验**三大核心机制，确保通信过程的安全性。

## 2.`SSH` 的应用

1. **远程登录**
   允许用户通过命令行界面（`CLI`）远程登录到服务器、路由器、嵌入式设备等，就像在本地操作一样执行命令。例如，管理员可通过 SSH 登录云服务器进行配置管理。
2. **远程命令执行**
   无需交互式登录，直接在本地通过SSH向远程设备发送单条或批量命令并获取结果，适合自动化脚本（如 Shell 脚本、Python 脚本）调用。
3. **安全文件传输**
   基于SSH协议衍生出的**SCP（Secure Copy）** 和**SFTP（SSH File Transfer Protocol）** 工具，可实现加密的文件上传 / 下载，替代不安全的 FTP 协议。
4. **端口转发（隧道）**
   通过SSH建立加密隧道，将其他协议（如 HTTP、`MySQL`）的流量封装在 SSH 连接中传输，绕过网络限制或增强非加密协议的安全性（例如，通过 SSH 隧道访问内网数据库）。

## 3.`SSH` 的工作简化流程

1. **建立连接**
   客户端向服务器发起 TCP 连接（默认端口 22），双方协商协议版本和加密算法（如 `AES`、`ChaCha20`）。
2. **身份认证**
   - **密码认证**：客户端输入服务器账户密码，密码经加密后传输（避免明文泄露）。
   - **密钥认证**（更安全）：
     - 客户端生成一对非对称密钥（公钥 + 私钥），将公钥上传至服务器的`~/.ssh/authorized_keys`文件。
     - 登录时，服务器用公钥加密随机信息，客户端用私钥解密并返回结果，验证通过则无需密码。
3. **数据传输**
   认证通过后，双方协商会话密钥，后续所有数据均通过该密钥加密传输，同时校验数据完整性，防止篡改。

## 4.常见应用场景

1. **服务器管理**
   运维人员通过 SSH 远程管理 Linux 服务器（如部署程序、查看日志、配置防火墙），几乎是服务器管理的标准方式。
2. **嵌入式设备调试**
   路由器、物联网设备（如树莓派）、工业控制器等常支持 SSH，方便开发者远程调试和配置。
3. **安全文件传输**
   用`scp`命令快速传输文件（如`scp localfile user@remote:/path`），或用`FileZilla`等工具通过 SFTP 图形化管理文件。
4. **绕过网络限制**
   通过 SSH 动态端口转发（如`ssh -D 1080 user@remote`）搭建 SOCKS 代理，实现科学上网或访问内网资源。
5. **自动化脚本**
   在 CI/CD 流程中，通过SSH自动向服务器部署代码；或用 Python 的`paramiko`库批量执行远程命令。

# `SSH`的使用工具

- **客户端工具**
  - 命令行：`Linux/macOS` 自带`ssh`命令，Windows 可通过 PowerShell（Win10+）、Git Bash、PuTTY 实现。
  - 图形化：Xshell、FinalShell、MobaXterm（支持多标签、密钥管理、SFTP 集成）。
- **服务器端**
  Linux 通过安装`openssh-server`，再通过`systemctl start sshd`启动服务。

# `SSH`的安装介绍

## 1.服务器和客户端安装

```bash
sudo apt install openssh-server  #下载安装ssh服务的服务器
sudo apt install openssh-client  #下载安装ssh服务的客户端
```

## 2.查看版本信息

```bash
ssh -V
```

## 3.启用ssh服务

```bash
sudo systemctl start ssh.service
sudo systemctl enable ssh.service
```

## 4.配置SSH服务

```bash
vi  /etc/ssh/sshd_config  #打开服务器配置文件,配置配置验证方式是否只能为密码还是密钥
```

[SSH远程登陆配置sshd_config文件详解-阿里云开发者社区](https://developer.aliyun.com/article/972993)

# `SSH`的连接命令

SSH（Secure Shell）是一种用于安全远程登录和管理服务器的网络协议。通过SSH，用户可以在本地计算机上安全地连接到远程服务器，并执行各种操作。以下是SSH远程登录的基本命令和一些常用选项。

```bash
ssh [options] [username]@[hostname_or_IP]
	
	[username] #如果本地用户名与远程服务器上的用户名相同，可以直接省略用户名
	[hoostname_or_IP]	#主机名或者IP地址
	[options]
		-p #指定端口号，默认为22
		-i #指定私钥文件（带绝对路径）比如ssh -i ~/.ssh/id_rsa user@192.168.1.100
		-J # 如果需要通过跳板机（中间服务器）访问目标服务器，可以使用  -J  选项，比如ssh -J user1@jump_host user2@target_host、
		-v #SSH客户端会输出详细的调试信息
```

# `SSH`的免密配置Ⅰ

- **本地主机生成密钥对**

  ```bash
  ssh-keygen  [option]
  		[option]
  			-t #type:指定密钥类型（如 rsa、dsa、ecdsa、ed25519）。默认为 rsa。
  			-f #file:指定生成的密钥文件名。默认为 ~/.ssh/id_rsa（对于 RSA 密钥）。
  			-C #customer：添加注释，通常用于描述密钥的用途或用户信息。
  			-N #设置私钥的密码（passphrase）。如果不设置密码，则私钥无密码保护。
  ```

- **添加公钥到远程主机**

  ```bash
  cat id_xxx.pub >> ~/.ssh/authorized_keys #生成密钥对后，需要将公钥添加到远程服务器的 ~/.ssh/authorized_keys 文件中，以便进行无密码登录。
  ```

- **配置本地主机的~/.ssh/config文件（可选)**

  ```bash
  #SSH（Secure Shell）的 config 文件是一个非常有用的用户配置文件，它允许用户自定义 SSH 客户端的行为，简化 SSH 连接过程，并提高安全性。默认情况下，SSH 客户端会读取位于用户主目录下的 ~/.ssh/config 文件。
  
  Host vir_linux  #Host:指定配置块适用的主机名或别名，为常用的服务器设置别名，从而简化 SSH 连接命令。例如，如果经常需要连接到 user@example.com，可以在 config 文件中设置一个别名 example，之后只需运行 ssh example 即可连接。
  
  HostName 192.168.65.113#指定实际的主机名或 IP 地址
  
  User linux #指定默认的用户名。
  
  IdentityFile ~/.ssh/id_rsa#指定使用的私钥文件路径
  ```

- **登录**

  ```bash
  ssh -i ~/.ssh/id_rsa linux@192.168.65.163
  ```

# `SSH`的免密配置Ⅱ

## 1.`SSHPASS`介绍

`sshpass`是一款**非交互式 SSH 密码认证工具**，核心作用是「绕过 SSH 手动输入密码的交互环节」，让` SSH/SSHFS/SCP `等依赖 SSH 认证的操作能在脚本、自动化任务中无人工干预执行。

## 2.核心定位

SSH 协议默认的密码认证是「交互式」的 —— 执行 `ssh user@ip` 后，终端会提示 `Password:` 并等待手动输入，这种交互模式在：

- 嵌入式开发板开机自动挂载 SSHFS；
- 脚本批量执行跨机操作；
- 自动化调试 / 部署任务；

中完全无法落地，而 sshpass 就是为解决这个问题而生：它能将密码通过命令行 / 文件传递给 SSH，实现「非交互式密码登录」。

## 3.有无的差异

|        无 `sshpass`（交互式）         |         有 `sshpass`（非交互式）          |
| :-----------------------------------: | :---------------------------------------: |
|        `ssh zhangg@10.0.3.100`        | `sshpass -p zg2025 ssh zhangg@10.0.3.100` |
| 终端提示：Password:（需手动输入密码） |   无提示，直接登录（密码通过 - p 传入）   |

## 4.核心原理
`sshpass`本质是「伪终端（`pty`）模拟工具」：

1. 它会先启动一个伪终端，替代用户的真实终端；
2. 当 SSH 程序在伪终端中输出 `Password:` 提示时，`sshpass`自动将预设的密码写入伪终端；
3. 完成密码输入后，`sshpass`退出，由 SSH 继续完成后续的认证和连接流程。

## 5.工具安装
### 1. 嵌入式开发板
```bash
# Debian/Ubuntu 系 ARM 系统（如全志/瑞芯微开发板）
sudo apt update && sudo apt install sshpass -y

# 若源中无预编译包（小众开发板），可手动编译：
# 1. 下载源码（官网：https://sourceforge.net/projects/sshpass/）
wget https://sourceforge.net/projects/sshpass/files/sshpass/1.09/sshpass-1.09.tar.gz
# 2. 解压编译
tar -zxvf sshpass-1.09.tar.gz && cd sshpass-1.09
./configure && make && sudo make install
```

### 2. 远程服务器

```bash
# Ubuntu/Debian
sudo apt install sshpass -y
```

## 6.核心用法

sshpass 的核心参数只有一个 `-p`（指定密码），结合不同 SSH 工具使用，最常用的场景如下：

### 1. 非交互式 SSH 登录

```bash
# 格式：sshpass -p 密码 ssh 用户名@远程IP [命令]
sshpass -p zg2025 ssh zhangg@10.0.3.100 "ls /home/zhangg"
# 作用：登录远程服务器，执行 ls 命令后退出（无手动输密码）
```

### 2. `SSHFS `免密挂载

```bash
sshfs \
  -o ssh_command='sshpass -p tc2016tc ssh -o StrictHostKeyChecking=no' \
 	zhangg@10.0.3.100:/home/zhangg/ModularizationTerminal/build/allwinner/Debug/bin \
  /tmp/zhangg
```

- `ssh_command='sshpass -p tc2016tc ssh ...'`：告诉 SSHFS「用 sshpass 传递密码，执行 SSH 连接」；
- 全程无交互，开发板执行脚本即可自动挂载。

### 3. `SCP `免密拷贝

```bash
# 格式：sshpass -p 密码 scp 本地文件 用户名@远程IP:远程路径
sshpass -p zg2025 scp /tmp/test.txt zhangg@10.0.3.100:/home/zhangg/
```

### 4. 从文件读取密码

避免密码明文出现在命令行（ps 命令能看到明文密码），可将密码写入文件：

```bash
# 1. 新建密码文件（开发板端）
echo "zg2025" > /root/ssh_pass.txt
chmod 600 /root/ssh_pass.txt  # 仅限root访问，避免泄露

# 2. 通过文件读取密码（替代-p）
sshpass -f /root/ssh_pass.txt ssh zhangg@10.0.3.100
# SSHFS 中适配
sshfs -o ssh_command='sshpass -f /root/ssh_pass.txt ssh -o StrictHostKeyChecking=no' ...
```

## 7.关键参数

### 1. 核心参数

|    参数     |               作用               |                     示例                      |
| :---------: | :------------------------------: | :-------------------------------------------: |
| `-p <密码>` | 直接指定密码（简单但有泄露风险） |          `sshpass -p zg2025 ssh ...`          |
| `-f <文件>` |     从文件读取密码（更安全）     |    `sshpass -f /root/ssh_pass.txt ssh ...`    |
|    `-e`     |  从环境变量 `SSHPASS` 读取密码   | `export SSHPASS=zg2025 && sshpass -e ssh ...` |

### 2. 踩坑点

|                          问题现象                           |                       根因 & 解决方案                        |
| :---------------------------------------------------------: | :----------------------------------------------------------: |
| `sshpass: Failed to run command: No such file or directory` | 1. ssh/sshpass 未安装（检查 `which sshpass`）；<br />2. 命令路径错误（用绝对路径：`/usr/bin/sshpass`） |
|           `Permission denied, please try again.`            | 1. 密码错误（核对 `-p` 后的密码）；<br />2. 远程服务器禁止密码登录（修改 `/etc/ssh/sshd_config` → `PasswordAuthentication yes`） |
|      密码明文出现在进程列表（`ps -ef | grep sshpass`）      |  改用 `-f`（文件读密码）或 `-e`（环境变量），避免 `-p` 明文  |
|               `Host key verification failed.`               | SSH 首次连接的主机密钥检查，需加 `-o StrictHostKeyChecking=no` |

# `SSH`的文件传输

`scp`（Secure Copy Protocol）是一个**基于 SSH（前提是安装了SSH服务）** 协议的命令行工具，用于在服务器之间或本地与远程服务器之间安全地复制文件和目录。它使用 SSH 加密传输数据，确保数据在传输过程中的安全性。

```bash
scp [选项] 源文件 目标文件
	[option]
		-r	#递归复制整个目录 
		-P	#指定远程服务器的SSH端口（默认为 22）
		-v	#启用详细模式，显示传输过程中的调试信息，有助于排查问题
		-C	#启用压缩，对传输的文件进行压缩，适合大文件或低带宽网络
```

# `SSH`的密钥权限

安全性`SSH`服务器需要确保只有授权用户可以访问和修改`.ssh`目录和`authorized_keys`文件。如果这些文件的权限设置过于宽松，可能会导致安全漏洞，例如：

- **未授权用户访问**
  如果`.ssh`目录或`authorized_keys`文件的权限设置允许其他用户读取或写入，攻击者可能会篡改这些文件，从而获得未经授权的访问权限。
- **中间人攻击**
  如果`.ssh`目录的权限设置不正确，攻击者可能会在其中放置恶意文件，导致中间人攻击。
- **权限检查机制**
  `SSH`服务器在处理公钥认证时，会检查`.ssh`目录和`authorized_keys`文件的权限。具体要求如下：
   - **`.ssh`目录**
      权限必须是 `XX0` （只有用户自己和同组用户可以读、写和执行）。其他用户不应有写权限，以防止未授权的修改。
   - **`authorized_keys 文件`**
      权限必须是` XX0 `（只有用户自己和同组用户可以读和写）。其他用户不应有读或写权限，以防止未授权的访问和修改。

SSH 服务器对`.ssh`目录和`authorized_keys`文件的权限非常敏感，这是出于安全考虑。如果这些文件的权限设置不正确，SSH服务器可能会拒绝公钥认证。通过确保`.ssh`目录的权限为`XX0` 和`authorized_keys`文件的权限为`XX0` ，可以避免这种问题。同时，确保这些文件的所有权属于正确的用户，以防止未授权的访问和修改。

# `SSH`密钥优先级

SSH 客户端在尝试连接时，会按照以下顺序查找密钥文件：

- 通过` -i `参数指定的密钥文件名。
- 在 `~/.ssh/config`文件中为特定主机配置的密钥文件。
- 默认的密钥文件**名**，如` ~/.ssh/id_rsa`  , `~/.ssh/id_ecdsa`等。

如果将自定义密钥文件直接放在 ~/.ssh 目录下，但没有通过` -i `参数或` ~/.ssh/config `文件明确指定，SSH 客户端不会自动使用该密钥文件，而是优先尝试默认的密钥文件名。

# `SSH`端口转发

## 1.简介

SSH 除了登录服务器，还有一大用途，就是作为加密通信的中介，充当两台服务器之间的通信加密跳板，使得原本不加密的通信变成加密通信。这个功能称为端口转发（port forwarding），又称 SSH 隧道（tunnel）。端口转发有两个主要作用：

1. 将不加密的数据放在 SSH 安全连接里面传输，使得原本不安全的网络服务增加了安全性，比如通过端口转发访问 Telnet、FTP 等明文服务，数据传输就都会加密。
2. 作为数据通信的加密跳板，绕过网络防火墙。

端口转发有三种使用方法：动态转发，本地转发，远程转发。下面逐一介绍。

## 2.动态转发(-D)

动态转发指的是，本机与 SSH 服务器之间创建了一个加密连接，然后本机内部针对某个端口的通信，都通过这个加密连接转发。它的一个使用场景就是，访问所有外部网站，都通过 SSH 转发。

动态转发需要把本地端口绑定到 SSH 服务器。至于 SSH 服务器要去访问哪一个网站，完全是动态的，取决于原始通信，所以叫做动态转发。可以用在电脑上开一个通用代理。浏览器设置好代理后，可以通过集中器访问家里**不同的设备**；每次不用指定固定目标。

```shell
ssh -D local-port tunnel-host -N 
-D  	    #表示动态转发
-N		    #表示这个 SSH 连接只进行端口转发,不登录远程 Shell，不能执行远程命令，只能充当隧道。
local-port   #是本地端口
tunnel-host  #是SSH服务器
```

如果本地端口是`2121`，那么动态转发的命令就是下面这样。

```shell
ssh -D 2121 tunnel-host -N 

#这种转发采用了 SOCKS5 协议。访问外部网站时，需要把 HTTP 请求转成 SOCKS5 协议，才能把本地端口的请求转发出去。
$ curl -x socks5://localhost:2121 http://www.example.com

#上面命令中，curl 的-x参数指定代理服务器，即通过 SOCKS5 协议的本地2121端口，访问http://www.example.com。
#如果经常使用动态转发，可以将设置写入 SSH 客户端的用户个人配置文件（~/.ssh/config）

DynamicForward tunnel-host:local-port 
```

## 3.本地转发(-L)

本地转发（local forwarding）指的是，SSH 服务器作为中介的跳板机，建立本地计算机与特定目标网站之间的加密连接。本地转发是在**本地计算机的 SSH 客户端**建立的转发规则。

它会指定一个本地端口（local-port），所有发向那个端口的请求，都会转发到 SSH 跳板机（tunnel-host），然后 SSH 跳板机作为中介，将收到的请求发到目标服务器（target-host）的目标端口（target-port）。

```shell
ssh -L local-port:target-host:target-port tunnel-host 

-L          #参数表示本地转发
local-port  #是本地端口
target-host #是想要访问的目标服务器
target-port #是目标服务器的端口
tunnel-host #是 SSH 跳板机。
```

举例来说，现在有一台 SSH 跳板机`tunnel-host`，我们想要通过这台机器，在本地`2121`端口与目标网站`www.example.com`的80端口之间建立 SSH 隧道，就可以写成下面这样。

```shell
ssh -L 2121:www.example.com:80 tunnel-host -N 
```

然后，访问本机的`2121`端口，就是访问`www.example.com`的80端口。

```shell
curl http://localhost:2121 

#本地端口转发采用 HTTP 协议，不用转成 SOCKS5 协议。
```

另一个例子是加密访问邮件获取协议 POP3。

```shell
ssh -L 1100:mail.example.com:110 mail.example.com 

#将本机的1100端口，绑定邮件服务器mail.example.com的110端口（POP3 协议的默认端口）。端口转发建立以后，POP3 邮件客户端只需要访问本机的1100端口，请求就会通过 SSH 跳板机（这里是mail.example.com），自动转发到mail.example.com的110端口。
```

上面这种情况有一个前提条件，就是`mail.example.com`必须运行 SSH 服务器。否则，就必须通过另一台 SSH 服务器中介，执行的命令要改成下面这样。

```shell
ssh -L 1100:mail.example.com:110 other.example.com 

#本机的1100端口还是绑`mail.example.com的110端口，但是由于mail.example.com没有运行SSH服务器，所以必须通过other.example.com中介。本机的 POP3 请求通过1100端口，先发给other.example.com的22端口（sshd 默认端口），再由后者转给mail.example.com，得到数据以后再原路返回。

#采用上面的中介方式，只有本机到other.example.com的这一段是加密的，other.example.com到mail.example.com的这一段并不加密。
#这个命令最好加上-N参数，表示不在 SSH 跳板机执行远程命令，让 SSH 只充当隧道。
#另外还有一个-f参数表示 SSH 连接在后台运行。
```

如果经常使用本地转发，可以将设置写入 SSH 客户端的用户个人配置文件（`~/.ssh/config`）。

```shell
Host test.example.com
LocalForward client-IP:client-port server-IP:server-port 
```

## 4.远程转发(-R)

远程转发指的是在远程 SSH 服务器建立的转发规则。它跟本地转发正好反过来。建立本地计算机到远程计算机的 SSH 隧道以后，本地转发是通过本地计算机访问远程计算机，而远程转发则是通过远程计算机访问本地计算机。它的命令格式如下。

```shell
ssh -R remote-port:target-host:target-port -N remotehost 

-R #表示远程端口转发
remote-port #是远程计算机的端口
target-host target-port #目标服务器及其端口
remotehost #是远程计算机。
```

远程转发主要针对内网的情况。下面举两个例子。

```shell
#内网某台服务器localhost在80端口开了一个服务，可以通过远程转发将这个80端口，映射到具有公网IP地址的my.public.server服务器的 8080端口，使得访问my.public.server:8080这个地址，就可以访问到那台内网服务器的80端口。
ssh -R 8080:localhost:80 -N my.public.server 

#上面命令是在内网localhost服务器上执行，建立从localhost到my.public.server的 SSH 隧道。运行以后，用户访问my.public.server:8080，就会自动映射到localhost:80。
```

第二个例子是本地计算机`local`在外网，SSH 跳板机和目标服务器`my.private.server`都在内网，必须通过 SSH 跳板机才能访问目标服务器。但是，本地计算机`local`无法访问内网之中的 SSH 跳板机，而 SSH 跳板机可以访问本机计算机。

由于本机无法访问内网 SSH 跳板机，就无法从外网发起 SSH 隧道，建立端口转发。必须反过来，从 SSH 跳板机发起隧道，建立端口转发，这时就形成了远程端口转发。跳板机执行下面的命令，绑定本地计算机`local`的`2121`端口，去访问`my.private.server:80`。

```shell
ssh -R 2121:my.private.server:80 -N local 
```

上面命令是在 SSH 跳板机上执行的，建立跳板机到`local`的隧道，并且这条隧道的出口映射到`my.private.server:80`。

显然，远程转发要求本地计算机`local`也安装了 SSH 服务器，这样才能接受 SSH 跳板机的远程登录。执行上面的命令以后，跳板机到`local`的隧道已经建立了。然后，就可以从本地计算机访问目标服务器了，即在本机执行下面的命令。

```shell
curl http://localhost:2121 
```

本机执行上面的命令以后，就会输出服务器`my.private.server`的 80 端口返回的内容。如果经常执行远程端口转发，可以将设置写入 SSH 客户端的用户个人配置文件（`~/.ssh/config`）。

```shell
Host remote-forward
  HostName test.example.com
  RemoteForward remote-port target-host:target-port 
```

完成上面的设置后，执行下面的命令就会建立远程转发。

```shell
ssh -N remote-forward # 等同于 $ ssh -R remote-port:target-host:target-port -N test.example.com 
```

## 5.具体实例

### 1.简易 VPN

VPN 用来在外网与内网之间建立一条加密通道。内网的服务器不能从外网直接访问，必须通过一个跳板机，如果本机可以访问跳板机，就可以使用 SSH 本地转发，简单实现一个 VPN。

```shell
ssh -L 2080:corp-server:80 -L 2443:corp-server:443 tunnel-host -N 
```

上面命令通过 SSH 跳板机，将本机的`2080`端口绑定内网服务器的`80`端口，本机的`2443`端口绑定内网服务器的`443`端口。

### 2.两级跳板

端口转发可以有多级，比如新建两个 SSH 隧道，第一个隧道转发给第二个隧道，第二个隧道才能访问目标服务器。首先，在本机新建第一级隧道。

```shell
ssh -L 7999:localhost:2999 tunnel1-host 
```

上面命令在本地`7999`端口与`tunnel1-host`之间建立一条隧道，隧道的出口是`tunnel1-host`的`localhost:2999`，也就是`tunnel1-host`收到本机的请求以后，转发给自己的`2999`端口。

然后，在第一台跳板机（`tunnel1-host`）执行下面的命令，新建第二级隧道。

```shell
ssh -L 2999:target-host:7999 tunnel2-host -N 
```

上面命令将第一台跳板机`tunnel1-host`的`2999`端口，通过第二台跳板机`tunnel2-host`，连接到目标服务器`target-host`的`7999`端口。

最终效果就是，访问本机的`7999`端口，就会转发到`target-host`的`7999`端口。

# `SSH`的参考链接

[Windows下通过ssh连接Linux_如何进行ssh连接-CSDN博客](https://blog.csdn.net/G_66_hero/article/details/97971023)

[Windows下生成ssh密钥，并用ssh免密访问Linux服务器_cmd生成ssh密钥-CSDN博客](https://blog.csdn.net/qq_43193386/article/details/120194085)

[SSH 端口转发 菜鸟教程](https://www.cainiaojc.com/ssh/ssh-port-forwarding.html)



# ——SSHFS远程挂载——

# `SSHFS`介绍及应用

`SSHFS`（`SSH Filesystem`）是基于 **FUSE（用户态文件系统）** 和 SSH 协议的轻量级网络文件系统，核心能力是通过 SSH 隧道将远程服务器的目录「挂载」到本地（或嵌入式开发板），实现如同访问本地文件一样操作远程文件。相比 NFS 无需搭建专用服务、相比 `SCP/rsync`支持实时读写，是嵌入式调试、跨机文件共享的高频方案（尤其适配「远程服务器→ARM 开发板」的调试场景）。

## 1.核心基础

### 1. 工作原理

`SSHFS` 基于 SSH 的 `SFTP`（`SSH File Transfer Protocol`）子协议，将远程文件的读写请求封装为 SSH 加密数据包，通过网络传输到远程服务器执行，结果返回给挂载端。核心优势：

- **免额外服务**
  仅需远程服务器开启 SSH 服务（默认 22 端口），无需搭建 NFS/Samba 等；
- **加密传输**
  依托 SSH 加密，数据传输安全（适合非内网场景）；
- **跨架构 / 系统**
  支持 x86 Linux、ARM Linux、Windows、macOS 等；
- **轻量灵活**
  临时挂载 / 按需卸载，无持久化配置成本。

### 2. 适用场景

|   场景类型   |                         典型使用场景                         |
| :----------: | :----------------------------------------------------------: |
|  嵌入式调试  | 远程服务器编译 ARM 程序后，挂载到开发板，避免手动 `SCP` 拷贝，保证路径 / 文件一致； |
| 跨机文件共享 |   本地访问云服务器 / 办公服务器的目录，无需频繁下载上传；    |
|   临时协作   | 多人共享同一远程目录，实时同步修改（替代网盘 / 邮件传文件）； |
|  低权限环境  | 无 root 权限搭建 NFS 时，SSHFS 仅需 SSH 账号即可（普通用户也能挂载）。 |

## 2.安装方法

### 1. 远程服务器

```bash
# Ubuntu/Debian
sudo apt update && sudo apt install sshfs -y
```

### 2. 嵌入式开发板

开发板需先安装适配 ARM 架构的 sshfs（优先用厂商源，无则手动编译）：

```bash
# 主流开发板（如友善之臂、全志）可直接安装
sudo apt install sshfs -y  # Debian/Ubuntu 系 ARM 系统
# 若源中无，需交叉编译 FUSE + SSHFS（略复杂，优先找预编译包）
```

### 3. 本地客户端

- Windows
  安装 WinFsp + SSHFS-Win（https://github.com/winfsp/sshfs-win）；
- macOS
  `brew install sshfs`（需先安装 Homebrew）。

## 3.语法与参数

### 1. 基础语法

```bash
sshfs [可选参数] [远程用户]@[远程IP]:[远程目录] [本地/开发板挂载点]
#示例
sshfs zhangg@10.0.3.100:/home/zhangg/ModularizationTerminal/build/allwinner/Debug./bin /tmp/zhangg  # 最简形式
```

### 2. 必选参数

`-o`表示`option`选项的意思。

|             参数格式              |                       作用 & 场景适配                        |
| :-------------------------------: | :----------------------------------------------------------: |
|      `-o ssh_command='xxx'`       | 自定义 SSH 登录命令（核心！适配嵌入式免交互登录）<br />示例：`ssh_command='sshpass -p 密码 ssh -o StrictHostKeyChecking=no'`→ 免手动输密码、跳过主机密钥检查（开发板首次连接不报错） |
|         `-o allow_other`          | 允许非挂载用户访问挂载目录（开发板多进程 / 多用户调试必需，默认仅挂载用户可访问）； 需先修改 `/etc/fuse.conf`，取消 `user_allow_other` 注释 |
|     `-o default_permissions`      | 继承远程目录的权限规则（保证开发板访问挂载文件的权限和远程服务器一致）； |
|         `-o uid=0,gid=0`          | 强制挂载目录的属主 / 属组为 root（开发板调试通常用 root，避免权限不足）； |
|        `-o charset=utf-8`         |      解决中文文件名乱码（嵌入式 Linux 常见编码问题）；       |
|      `-o ConnectTimeout=10`       | SSH 连接超时时间（避免网络异常时挂载命令卡死，建议设 10-30 秒）； |
| `-o UserKnownHostsFile=/dev/null` | 不生成 SSH 主机密钥文件（开发板无持久化存储时，避免每次挂载生成冗余文件）； |

### 3. 进阶参数

|                参数                 |                           作用                            |
| :---------------------------------: | :-------------------------------------------------------: |
|           `-o cache=yes`            |  开启本地缓存（提升小文件读写性能，调试场景建议开启）；   |
|        `-o compression=yes`         | 开启数据压缩（降低网络带宽占用，低速网络 / 大文件推荐）； |
| `-o IdentityFile=/root/.ssh/id_rsa` |  用 SSH 密钥登录（替代明文密码，更安全，避免密码泄露）；  |
|           `-o port=2222`            | 指定非默认 SSH 端口（远程服务器 SSH 端口非 22 时使用）；  |

# `SSHFS`使用流程

以「远程服务器（10.0.3.100）→ ARM 开发板」为例，覆盖挂载→验证→调试→卸载全流程：

## 1.前置准备

1. 远程服务器开启 SSH 服务，确保开发板能 ping 通 10.0.3.100；

2. 开发板安装 `sshpass`（免密码登录依赖）

   ```bash
   sudo apt install sshpass -y
   ```

3. 开发板修改 fuse 配置（允许`allow_other`）

   ```bash
   sudo vim /etc/fuse.conf
   # 取消注释：user_allow_other
   sudo chmod 644 /etc/fuse.conf  # 确保配置生效
   ```

## 2.编写挂载脚本

新建 `/root/mount_remote.sh`（开发板端），避免手动输入冗长命令：

```bash
#!/bin/bash
# 1. 卸载旧挂载（避免重复挂载报错）
umount /tmp/zhangg >/dev/null 2>&1
# 2. 创建挂载点（不存在则创建）
mkdir -p /tmp/zhangg
# 3. 执行 SSHFS 挂载（适配嵌入式）
/usr/bin/sshfs \
  -o ssh_command='sshpass -p zg2025 ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -o ConnectTimeout=10' \
  -o allow_other,default_permissions,charset=utf-8,uid=0,gid=0,cache=yes \
 zhangg@10.0.3.99:/home/zhangg/menggu/sg698/debian/tmp/usr/bin \
  /tmp/zhangg
# 4. 验证挂载结果
if [ $? -eq 0 ]; then
  echo -e "\033[32m挂载成功！\033[0m 开发板/tmp/zhangg 映射远程服务器目录："
  ls -l /tmp/tc  # 列出挂载文件，确认同步
else
  echo -e "\033[31m挂载失败！\033[0m 检查：1.远程IP/密码 2.SSH服务 3.远程路径是否存在"
  exit 1
fi
```

## 3.执行挂载

```bash
# 开发板端赋予脚本执行权限并运行
chmod +x /root/mount_remote.sh
/root/mount_remote.sh
```

## 4.调试场景

挂载成功后，开发板 `/tmp/zhangg` 目录与远程服务器的 `/home/zhangg/.../bin` 实时同步，调试流程简化：

1. 远程服务器（VS Code SSH 连接）编译代码（带`-g -O0`）

   ```bash
   arm-linux-gnueabihf-g++ -g -O0 -o build/allwinner/Debug/bin/taskManager 源码文件.cpp
   ```

2. 开发板直接在挂载目录启动 gdbserver（路径 / 文件与远程完全一致）：

   ```bash
   cd /tmp/zhangg  # 切换到挂载目录（与远程服务器 cwd 逻辑一致）
   gdbserver 192.168.65.148:12000 ./taskManager
   ```

3. VS Code 启动调试
   `launch.json` 无需修改，断点 / 变量调试 100% 匹配（文件实时同步，无拷贝误差）。

## 5.卸载挂载

```bash
# 正常卸载
umount /tmp/zhangg

# 若提示「资源忙」（文件被占用），先杀进程再卸载
fuser -k /tmp/zhangg && umount /tmp/zhangg

# 强制卸载（极端情况，谨慎使用）
umount -lf /tmp/zhangg
```

# `SSHFS`常见问题

|                  问题现象                  |                       根因 & 解决方案                        |
| :----------------------------------------: | :----------------------------------------------------------: |
|        挂载提示 `Permission denied`        | 1. 远程服务器密码错误（核对 `sshpass -p` 后的密码）；<br />2. 远程服务器禁止密码登录：修改 `/etc/ssh/sshd_config` → `PasswordAuthentication yes`，重启 sshd；<br />3. 未开启 `user_allow_other`：重新修改 `/etc/fuse.conf` 并重启挂载；<br />4. 开发板 sshfs 无执行权限：`chmod +x /usr/bin/sshfs` |
|      挂载成功但 `ls /tmp/zhangg` 为空      | 1. 远程服务器路径错误（核对 `/home/zhangg/.../bin` 是否存在）；<br />2. 远程目录权限不足：`chmod 755 /home/zhangg/.../bin`（保证读权限）；<br />3. `SSHFS` 缓存延迟：`umount` 后重新挂载，或加 `-o cache=no` |
| 开发板执行挂载文件提示 `exec format error` | 远程服务器编译的程序是` x86 `架构（误用工 `gcc `而非 arm-linux-gnueabihf-gcc）；重新交叉编译 ARM 版本 |
|             挂载后文件读写卡顿             | 1. 网络带宽低：加 `-o compression=yes` 开启压缩；<br />2. 缓存未开启：加 `-o cache=yes`；<br />3. SSH 连接不稳定：检查开发板 / 服务器网络（优先有线连接） |
|         卸载提示 `device is busy`          | 1. 当前终端在挂载目录下：`cd /` 后再卸载；<br />2. 程序 / 进程占用挂载文件：`fuser -k /tmp/zhangg` 杀进程后卸载；<br />3. 开发板内核锁死：重启开发板后强制卸载 |
|               中文文件名乱码               | 挂载时未指定编码：加 `-o charset=utf-8` 参数，卸载后重新挂载 |

# `SSHFS` vs 其他方案

|  方案   |          核心优势          |          核心劣势          |              适配场景               |
| :-----: | :------------------------: | :------------------------: | :---------------------------------: |
| `SSHFS` | 免搭建服务、SSH 加密、轻量 |   性能低于 NFS、依赖网络   | 临时调试、跨机轻量共享、无 NFS 权限 |
|  `NFS`  |       性能高、延迟低       | 需搭建服务、无加密、内网用 |  长期开发、高频文件读写、内网环境   |
| `rsync` |     离线可用、增量同步     |     非实时、需手动触发     |   开发板离线调试、网络不稳定场景    |
|  `SCP`  |       简单、单次拷贝       |  无实时同步、频繁拷贝易错  |       单次文件传输、临时调试        |

# —— samba网络映射——

[Windows上将linux目录映射网络驱动器 - liqinglucky - 博客园 (cnblogs.com)](https://www.cnblogs.com/liqinglucky/p/samba.html)

```shell
[public] #windows上填写的需要映射的目录名称别名
        path = /fdsk #需要映射的目录
        public = yes
        writeable = yes
        browseable = yes
        guest ok = yes
```

