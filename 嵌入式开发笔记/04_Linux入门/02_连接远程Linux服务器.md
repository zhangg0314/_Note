# ——SSH——

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

# `SSH`的免密配置

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

# `SSH`的文件传输

scp（Secure Copy Protocol）是一个**基于 SSH（前提是安装了SSH服务）** 协议的命令行工具，用于在服务器之间或本地与远程服务器之间安全地复制文件和目录。它使用 SSH 加密传输数据，确保数据在传输过程中的安全性。

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

# `SSH`的参考链接

[Windows下通过ssh连接Linux_如何进行ssh连接-CSDN博客](https://blog.csdn.net/G_66_hero/article/details/97971023)

[Windows下生成ssh密钥，并用ssh免密访问Linux服务器_cmd生成ssh密钥-CSDN博客](https://blog.csdn.net/qq_43193386/article/details/120194085)