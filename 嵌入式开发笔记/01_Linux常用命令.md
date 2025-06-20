# SSH

## SSH安装

### 服务器和客户端安装

```bash
sudo apt update        #更新数据
sudo apt upgrade        #更新软件
sudo apt install openssh-server  #下载安装ssh服务的服务器
sudo apt install openssh-client  #下载安装ssh服务的客户端
```

### 查看版本信息

```bash
ssh -V
```

### 启用ssh服务

```bash
sudo systemctl start sshd
sudo systemctl enable sshd
```

### 配置SSH服务

```bash
vi  /etc/ssh/sshd_config  #打开服务器配置文件
```

## SSH远程连接

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

## SSH免密登录

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
  #SSH（Secure Shell）的 config 文件是一个非常有用的配置文件，它允许用户自定义 SSH 客户端的行为，简化 SSH 连接过程，并提高安全性。默认情况下，SSH 客户端会读取位于用户主目录下的 ~/.ssh/config 文件。
  
  Host vir_linux  #Host:指定配置块适用的主机名或别名，为常用的服务器设置别名，从而简化 SSH 连接命令。例如，如果经常需要连接到 user@example.com，可以在 config 文件中设置一个别名 example，之后只需运行 ssh example 即可连接。
      HostName 192.168.65.113#指定实际的主机名或 IP 地址
      User linux #指定默认的用户名。
      IdentityFile ~/.ssh/id_rsa#指定使用的私钥文件路径
  ```

- **登录**

  ```bash
  ssh -i ~/.ssh/id_rsa linux@192.168.65.163
  ```

## SSH传输文件

 scp（Secure Copy Protocol）是一个**基于 SSH（前提是安装了SSH服务）** 协议的命令行工具，用于在服务器之间或本地与远程服务器之间安全地复制文件和目录。它使用 SSH 加密传输数据，确保数据在传输过程中的安全性。

```bash
scp [选项] [源文件] [目标文件]
	[option]
		-r	#递归复制整个目录 
		-P	#指定远程服务器的SSH端口（默认为 22）
		-v	#启用详细模式，显示传输过程中的调试信息，有助于排查问题
		-C	#启用压缩，对传输的文件进行压缩，适合大文件或低带宽网络
```

## SSH密钥权限

​	安全性SSH 服务器需要确保只有授权用户可以访问和修改 .ssh  目录和 authorized_keys  文件。如果这些文件的权限设置过于宽松，可能会导致安全漏洞，例如：

- **未授权用户访问**
  如果 .ssh 目录或 authorized_keys 文件的权限设置允许其他用户读取或写入，攻击者可能会篡改这些文件，从而获得未经授权的访问权限。
- **中间人攻击**
  如果 .ssh 目录的权限设置不正确，攻击者可能会在其中放置恶意文件，导致中间人攻击。
- **权限检查机制**
  SSH 服务器在处理公钥认证时，会检查 .ssh目录和 authorized_keys   文件的权限。
  具体要求如下：
  **.ssh   目录**： 权限必须是 XX0 （只有用户自己和同组用户可以读、写和执行）。其他用户不应有写权限，以防止未授权的修改。
  **authorized_keys 文件**：权限必须是 XX0 （只有用户自己和同组用户可以读和写）。其他用户不应有读或写权限，以防止未授权的访问和修改。

​	SSH 服务器对   .ssh   目录和   authorized_keys   文件的权限非常敏感，这是出于安全考虑。如果这些文件的权限设置不正确，SSH 服务器可能会拒绝公钥认证。通过确保   .ssh   目录的权限为  XX0   和   authorized_keys   文件的权限为  XX0  ，可以避免这种问题。同时，确保这些文件的所有权属于正确的用户，以防止未授权的访问和修改。

## 默认密钥文件的优先级

SSH 客户端在尝试连接时，会按照以下顺序查找密钥文件：

- 通过 -i 参数指定的密钥文件。
- 在 ~/.ssh/config文件中为特定主机配置的密钥文件。
- 默认的密钥文件，如 ~/.ssh/id_rsa   ~/.ssh/id_ecdsa等。

如果将自定义密钥文件直接放在 ~/.ssh 目录下，但没有通过 -i 参数或 ~/.ssh/config 文件明确指定，SSH 客户端不会自动使用该密钥文件，而是优先尝试默认的密钥文件。

## 参考链接

[Windows下通过ssh连接Linux_如何进行ssh连接-CSDN博客](https://blog.csdn.net/G_66_hero/article/details/97971023)

[Windows下生成ssh密钥，并用ssh免密访问Linux服务器_cmd生成ssh密钥-CSDN博客](https://blog.csdn.net/qq_43193386/article/details/120194085)

# Linux

## 1.Linux常用命令

在Shell命令行中，空格是分隔命令参数的默认分隔符，所以当输入`cd 11 软件`时bash会把它解析为两个参数:`11和软件`，而不是一个完整的文件夹名`11 软件 `，解决办法就是用双引号括起成一个整体，或者给空格加反斜杠转义符。

### 1.文件相关命令

- **创建文件**

  ```bash
  vi a.txt 				  #默认打开a.txt，没有则创建再打开
  
  touch <FILENAME>           #创建/打开文件
  ```
  
- **查找文件或目录**

  ```bash
  find [搜索路径] [选项] [正则表达式]
  	[选项]
  		-name	#根据文件名查找		eg:find ./ -name = *.txt
  		-type	#根据文件类型查找       eg:find -./ type = l
  		-size	#根据文件大小查找-+n：大于 n   -n：小于 n     n：等于 n
  		-maxdepth n #搜索目录递归的深度为n，省略默认全盘搜索
  
  #组合条件:查找当前目录及其子目录下所有属于用户 username 或大小大于 1MB 的文件
  find . -type f \( -user username -o -size +1M \)
  ```

- **编辑文件**

  ```bash
  vi example.txt		#在shell模式下,键入vi及需要编辑的文件名,即可进入vi. 
  
  vi +5 example.txt	#如果需要在进入vi编辑界面后，将光标置于文件的第n行，则在vi命令后面加上“+n” 参数即可
  ```

- **查看文件内容**

  ```bash
  cat /etc/sysconfig/network	#cat <FILENAME>,将文件内容合并到标准输出，即输出文件内容
  	[option]
  		-n #查看行号信息
  ```

- **拷贝三件套**

  ```bash
  cp <SOURCE> <DEST>             #复制文件或文件夹
  	[option]    -r(--recusive) #递归拷贝子目录
              	-f(--FORCE)    #强制拷贝，忽略提示信息
  
  
  mv <SOURCE> <DEST>       #重命名或者移动文件或文件夹
  
  
  rm <FILENAME>			#删除文件               
     [option]  -r          #递归删除删除文件夹以及文件夹里的东西		
               -f          #不提示直接强制删除		
               -i          #删除前提示是否删除   
  ```
  
- **目录操作**

  ```bash
  pwd		#显示当前工作区所在路径的绝对路径
  
  
  ls -altr               	   #显示当前文件夹下的文件及文件夹
  	[option]	-a		  #显示所有文件及文件夹，包括隐藏文件
  			    -l		  #以列表，即显示文件名，大小，日期，权限等方式显示文件及文件夹
  			    -t		  #按修改时间排序，最近修改的排第一
  			    -r		  #如果进行排序，则倒序显
  cd  [Directory]             #切换当前工作路径,缺省Directory默认回到home目录
  
  
  mkdir <DIRECTORYNAME>      	   #创建文件夹        
  	[option]    -p(--parent)   #级联创建文件夹
  
  
  rmdir <DIRECTORYNAME>     	   #删除空目录        
  	[option]    -p             #递归删除文件夹，先删子目录，之后若父目录也空则也删除，故p后面的路径要包含父目录名和子目录名  
  ```
  
- **文件类型**

  ```bash
  file [option] 文件名
  	[option]
  		-b#省略文件名，只显示文件类型 
  		-f#指定一个文件，文件中包含要检查的文件名列表
  		-L#符号链接进行检查，而不是检查符号链接指向的文件
  		-z#尝试检查压缩文件的内容类型
  
  #假设有一个文件filelist.txt，内容如下:
  example.txt
  example.jpg
  example.mp3
  
  file -f filelist.txt
  
  example.txt: ASCII text
  example.jpg: JPEG image data, JFIF standard 1.01
  example.mp3: Audio file with ID3 version 2.2.0, contains: MPEG ADTS, layer III, v1, 128 kbps, 44.1 kHz, Stereo
  ```
  
- **文件权限**

  ```bash
  umask             #获取系统当前权限掩码，文件实际权限为创建文件时赋予的权限-权限掩码
  ```
  

### 2.系统相关命令

- **关机重启**

  ```bash
  sudo shutdown -h now/时间 [关机提示信息] #立即/定时关机
  sudo shutdown -r now/时间 [重启提示信息] #立即/定时重启
  sudo reboot                             #立即重启
  ```

- **系统主机信息**

  ```bash
  uname -n #显示主机名
  
  uname -r #显示内容核版本
  
  uname    #显示内核名，如Linux或者Unix
  
  unmae -a #显示更加详细的信息
  
  cat /etc/os-release  #查看操控操作系统发行版本
  ```

- **终端**

  ```bash
  clear             #清除终端所有打印的信息
  
  history 		 #查询历史命令
  
  !c       		 #执行上一个以c开头的命令
  
  !g      		 #执行上一个以g开头的命令，常为gcc
  
  table键  		#自动补齐
  ```

### 3.权限相关命令

- **切换用户**

  ```bash
  su [-l]  <用户名>	#切换到目标用户，加-l(环境PATH也会变)，看$前面是~还是username，如果是~表示切换了当前用户且当前环境也是该用户的，否则只是切换了用户并未切换环境，环境没变，有些有权限的操作不能使用。
  
  su -c <命令>  <用户名>	#切换到目标用户执行一下该命令再切换回当前用户
  
  exit                    #退出目标用户到自己的用户下
  ```

- **添加用户**

  ```bash
  sudo adduser <用户名>                  #添加用户
  ```

- **用户密码**

  ```bash
  sudo vi /etc/passwd                 #查看用户账户信息的文件
  #passwd字段格式
  #username:password:UID:GID:GECOS:home_directory:shell
  #GECOS:用户信息字段，通常包含用户的真实姓名、电话号码、房间号码等信息。该字段主要用于描述用户，但具体格式和内容可能因系统而异。
  
  passwd                               #修改当前用户密码
  
  sudo passwd <用户名>                  #修改指定用户名密码
  ```

- **用户权限**

  ```bash
  vi      /etc/group                    #查看组
  
  chmod <八进制> <FILENAME>              #修改文件权限
  
  chmod [u | g | o | a(all)]  [+ | - | =]  [r | w | x] <FILENAME>
  ```

### 4.网络相关命令

- **网络测速**

  ```bash
  iperf3 -s #在服务器上运行,这将启动一个监听默认端口（5201）的服务器
  
  iperf3 -c <服务器IP地址>	#在客户端运行,这将连接到服务器并开始测试
  
  [option]
  	-p	#port:指定端口号（默认为 5201）。
  	-t	#time:设置测试持续时间（默认为 10 秒）。
  	-i	#interval:设置报告间隔时间。
  	-P	#启用并发连接数。
  	-u	#UDP:使用 UDP 协议进行测试。
  	-b	#bind width在 UDP 测试中指定目标带宽。
  	
  iperf3 -c 192.168.1.100 -u -b 10M#这将使用 UDP 协议，目标带宽为 10 Mbps
  
  
  Connecting to host localhost, port 5201
  [  5] local 127.0.0.1 port 33898 connected to 127.0.0.1 port 5201
  [ ID] Interval           Transfer     Bitrate         Retr  Cwnd
  [  5]   0.00-1.00   sec  4.06 GBytes  34.8 Gbits/sec    0   3.12 MBytes       
  [  5]   1.00-2.00   sec  4.07 GBytes  35.0 Gbits/sec    0   3.43 MBytes       
  [  5]   2.00-3.00   sec  4.07 GBytes  34.9 Gbits/sec    0   3.43 MBytes       
  [  5]   3.00-4.00   sec  4.04 GBytes  34.7 Gbits/sec    0   3.43 MBytes   
  ```

- **建立TCP/UDP连接**

  ```bash
  nc  [option] <ip> <port>        #建立tcp或者udp连接，nc即netcat
  	[option]      -u            #使用UDP协议,默认使用TCP协议
  ```

- **ifconfig**

  ```bash
  #查看所有网络接口信息
  ifconfig
  
  #查看特定网络接口信息
  ifconfig eth0
  
  #配置网络接口
  sudo ifconfig eth0 192.168.1.100 netmask 255.255.255.0#静态IP
  sudo ifconfig eth0 broadcast 192.168.1.255
  
  sudo ifconfig eth0 up   # 启用 eth0 接口
  sudo ifconfig eth0 down # 禁用 eth0 接口
  
  ifconfig -a #这将显示所有网络接口（包括未激活的）的详细信息。
  ```

- **ip命令**
  在现代 Linux 系统中，`ip` 命令逐渐取代了 `ifconfig` 的部分功能。

  ```bash
  ip a #查看网络接口信息
  
  ip addr show eth0#查看特定网络接口信息
  
  sudo ip addr add 192.168.1.100/24 dev eth0#设置 IP 地址,静态IP
  sudo ip addr del 192.168.1.100/24 dev eth0#删除 IP 地址
  sudo ip link set eth0 up   # 启用 eth0 接口
  sudo ip link set eth0 down # 禁用 eth0 接口
  ```

### 5.进程相关命令

- **静态查看进程**

  ```bash
  ps  [option]           #process status，用于查询进程状态及进程信息,查到的进程可能处于运行态，也可能处于僵尸态等其他状态。
  	
  	[option]	-elf   #更适合系统管理员查看进程的优先级、状态和启动时间等详细信息。
  			    -aux   #更适合普通用户查看进程的CPU和内存占用情况，以及快速定位资源占用较高的进程。
  ```

- **动态查看进程**

  ```bash
  top [option]	#监控Linux系统的状况
  	-d	n	   #表示top页面更新的时间间隔为n秒，默认为5秒
  	-p  pid	   #指定特定进程的pid号进行观察
  ```

- **查看pid号**

  ```bash
  ps
  
  top
  
  pidof	#Linux系统中用来查找正在运行进程的进程号(pid)的工具， pidof命令用于查找指定名称的进程的进程号，查到的进程一定处于运行态。
  ```

- **杀死进程**

  ```bash
  killall   #杀死同一进程组内的所有进程，其允许指定要终止的进程的名称，而非PID
  
  kill -9 PID #杀死指定进程号的进程
  ```

### 6.输入输出命令

- **终端回显**

  ```bash
  echo	[option]	<输出内容>   
  		[option]
  			-e(enable)  	#使能转义
               -E(disable)	 #默认情况
  
  echo "hello world"  	  #默认带换行符
  
  echo -n "hello world"	  #不带换行符
  
  echo "hello world\n"	  #默认带换行符，\n一样被打印出来
  
  echo -e "hello world\n"   #默认带换行符，\n也被翻译成换行。
  
  
  a = 5
  echo $a   #取a变量的值,弱引用，想当于"$a"
  
  echo '$a' #直接打印$a，强引用
  ```

- **重定向**
  
  ```bash
  #输出重定向
  ls > log    #打开/新建log，清空之前的再写,不加n默认是标准输出重定向到log
  
  ls >> log   #打开/新建log，追加
  
  ls file 2 >> log      #标准出错重定向，但标准输出又不会重定向
  
  ls file > log 2>&1    #标准输出重定向到log，标准出错重定向到标准输出
  
  ls file &> log        #与ls file >log 2>&1等价
  	
  #输入重定向
  cat           #从标准输入输入
  
  cat file      #把file内容利用标准输入读取并输出到标准输出上
  
  cat < file    #与cat file等价
  
  cat << number #<<后面的number其实是数字作为分隔符，当输入遇到指定数值number时停止输入，打印所有之前的输入的数据。
  ```
  
### 7.通配符

  ```bash
  *.c        #*匹配任意长度的字符串，即*可以代表任意长度的任意字串
  
  ?.c        #?匹配一个长度的字符串，即?可以代表任意一个字符
  
  [1,3,4].c  #匹配?.c其中的1，3，4，即1.c，3.c，4.c
  
  [1-5].c    #匹配?.c其中的1-5，即1，2，3，4，5
  
  [^1-5].c   #匹配?.c其中的除了1-5.c之外的。
  ```
### 8.管道命令

```bash
#管道前的命令一定要有输出结果才能传给管道后的命令

cat file | wc -l 

wc [option]
	[option]  -l  #统计有多少行内容
         	  -w  #多少个单词
              -c  #多少个字符
```

### 9.命令置换

```bash
a = `ls` #不让ls的结果输出到终端，而是把结果存到变量a中

echo $a

ls pwd   #系统以为ls列出名为pwd的文件

ls `pwd` #先执行pwd，再ls就是ls当前目录的内容了
```

  ### 10.环境变量相关命令 

- **在当前shell环境中执行脚本或加载配置文件**
  
  ```bash
  source 文件名
  	source ~/.bashrc #加载环境变量
  	source .env      #加载.env文件
  
  ./script.sh			#直接执行脚本（在新的shell中执行）
  source script.sh 	 #source执行（在当前shell中执行）
  
  
  #创建一个包含环境变量的文件env.sh
  export PROJECT_ROOT="/path/to/project"
  export API_KEY="your-api-key"
  #加载环境变量
  source env.sh
  #验证环境变量是否生效
  echo $PROJECT_ROOT
  echo $API_KEY
  ```

## 2.VIM编辑器

- **移动**

  ```bash
  h j k l	#分别为左移一个字符，下移一行，上移一行，右移一个字符
  
  
  #2.行内移动：
   w	#向后一个单词
   b	#向前一个单词
   0	#行首    
   ^	#行首非空白字符 
   $	#行尾
   
   #3.行间移动：
  	gg	#文件首部
  	G	#文件尾部
  	数字n  gg或者G #移动到第n行 
      gg=G#对齐缩进全文
    	=	#对齐缩进当前光标所在行，也可以缩进选中的多行
    	
  #4.页间移动：
  	ctrl+b	#向上翻页   
      ctrl+f（forword）#向后翻页
      直接瞬移到文件的（H M L ）头 中 尾
  ```

- **复制删除剪切**

  ```bash
  #1.删除剪切复制：
   	#1)一个字符：
  		x 剪切一个字符
  	#2)n行：
  		[n] dd 剪切n行  
  		[n] yy 复制n行
           [n] x  剪切n个字符
  	#3)粘贴:
  	    p#用于vim编辑器内部之间的粘贴
          
          shift + insert
          ctr+shift+v#用于外部的ctr+c复制过来的粘贴到vim编辑器内部
  	#4)行内：
  		y或者d 0  #复制或者剪切光标处到行首的那一部分
           y或者d $  #复制或者剪切光标处到行尾的那一部分
  
   	#5)块：
  		:n,m d或者y  #底行模式
      # 6)删除一个单词：
            d w
  ```



****

```bash

#2.替换：
   	   r替换一个字符 
        R进入替换模式，可以猛猛替换
#3.查找：
		/str：查找str，n高亮显示下一个，N显示上一个
		:n，m s/str/det_str/g c（确认替换）全部替换
        :% s/str/des_str/g 全文替换
8.分屏命令：
		:sp [文件名] 横向
		:vsp [文件名]竖向
		Ctrl+h j k l移动到哪个窗口
9.行号显示:
		:setnu 设置行号  
        :setnonu取消行号
10.定位行：
         vi 文件名 +n：打开已有文件，并且光标初始在第n行的位置
         :n  光标定位到第n行//好处在于，在几千行的代码中只需要一个命令就能直接定位，而不需要鼠标慢慢的滑
11.文件保存：
          q：退出
          w:保存
          q！：不保存修改的内容直接退出
          w file：另存为文件file
          r file：读取file文件内容到光标处
12.可视模式：
          v：进入可视模式，可以用上下左右选中
          V：直接选中光标所在行的一整行
13.撤销恢复：
          u：撤销上一个命令
          ctrl+r：恢复被撤销的命令
```

## 	3.软件包管理

### 		1.软件包类型

- 二进制包：系统可以直接安装拿来用的包格式。

- 源码包：需要经过编译生成二进制包才可以安装。


```shell
file  test.c
file  xxx.deb
#软件包包名/文件：查看软件包/文件类型
```

### 		2.在线软件包管理

```shell
#更新软件源
1.修改/etc/apt/source.list
2.sudo apt-get update
3.图形界面修改


sudo apt-get install 包名  #下载包名并安装
sudo apt-get --purge remove 包名  #删除包名以及依赖关系
sudo apt-cache depends 包名  #查看包的依赖关系


#apt-get在安装过程中提示用户确认是否继续操作，而 Docker 构建过程是自动化的，无法手动输入确认。解决方法在 Docker 构建过程中，需要确保命令能够自动运行，而不会提示用户输入。可以通过以下方法解决：方法1：使用   -y   选项自动确认在   apt-get install   命令中添加   -y   选项，这样可以自动确认安装操作，无需手动输入。

RUN apt-get update && apt-get install -y iputils-ping iproute2 net-tools
```

### 		3.离线软件包管理

```shell
sudo dpkg -i(install) 包名 #安装（包名格式要全）
sudo dpkg -r(remove) 包名 #移除（只要包名也可以）


dpkg -s(status) 包名 #显示安装状态
sudo dpkg -P(purge) 包名 #移除包以及配置文件

dpkg -L(list) 包名 #列出软件包的清单
```

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

## 5.参考链接

https://www.kancloud.cn/digest/linux-world/145282

# Git

## 1.Git介绍

​	Git 是一个开源的分布式版本控制系统（ Distributed Version Control System，简称 DVCS ），用于高效地管理项目版本。它能够有效、高速地处理从小到大的项目版本管理，追踪项目从开始到结束的整个过程。

​	版本控制（ Version Control ）是软件配置管理的一项内容，主要对程序、文档等项目制品的版本变更进行管理。它允许开发者记录、追踪和管理代码或其他文件的历史变化，以便在需要时能够恢复到以前的版本或查看历史记录。版本控制是团队协作开发的桥梁，有助于多人协作同步进行大型项目开发。版本控制系统是团队协作开发的桥梁，它通过分支管理、**自动合并（无需传统的人工拿着两个温度看哪里添加哪里删除来修改合并）和冲突解决**（如果多个开发人员修改了同一个文件，版本控制系统会标记冲突，开发人员可以手动解决冲突。）、历史记录和追溯、代码审查等功能，支持多人协作同步进行大型项目的开发。它不仅提高了开发效率，还确保了代码质量和项目的稳定性。

## 2.集中式vs分布式

- **集中式版本控制系统**
  版本库集中存放在中央服务器。最大的毛病就是必须联网才能工作。
- **分布式版本控制系统**
  没有“中央服务器”，每个人的电脑上都是一个完整的版本库。你在自己电脑上改了文件A，你的同事也在他的电脑上改了文件A，这时，你们俩之间只需把各自的修改推送给对方，就可以互相看到对方的修改了。安全性要高很多，因为每个人电脑里都有完整的版本库，某一个人的电脑坏掉了不要紧，随便从其他人那里复制一个就可以了，这是因为每个人都需要在本地创建版本库，并且把修改提交到版本库了才可以推送即push前都要先commit，所以相当于强制性要求了每个人在推送前都要在本地备份一样。而集中式版本控制系统的中央服务器要是出了问题，所有人都没法干活了。分布式版本控制系统通常也有一台充当“中央服务器”的电脑，但这个服务器的作用仅仅是用来方便“交换”大家的修改，没有它大家也一样干活，只是交换修改不方便而已。

## 3.Git的作用和目的

1. **版本控制**：Git允许开发者跟踪代码的版本历史，可以回溯到任何时刻的代码状态。这对于调试、修复错误以及恢复代码都非常有用。
2. **分支管理**：Git支持创建多个分支，使得团队成员可以并行地开发不同的功能和修复不同的bug。<u>每个分支都可以独立地进行开发和测试</u>，最后再将分支合并到主分支上。这种分支策略提高了开发效率和灵活性。
3. **合作开发**：Git提供了远程仓库的功能，使得团队成员可以共享和合并代码。通过Git，开发者可以轻松地将代码推送到共享仓库，并进行同步，从而促进了团队协作和代码共享。
4. **撤销和回退**：Git允许开发者撤销之前的提交，回退到之前的版本。这对于修复错误或不需要的更改非常有用，保证了代码的稳定性和可维护性。
5. **快速部署**：Git可以轻松部署代码到生产环境。通过创建发布分支或打标签，开发者可以准确地控制代码的部署版本，提高了部署的效率和准确性。
6. **高度可靠性和安全性**：Git的分布式架构意味着每个开发者都有一份完整的代码库，即使服务器出现故障，开发者仍然可以继续工作。此外，Git使用哈希算法来标识每个版本，确保每个版本的唯一性和完整性，提高了代码的安全性和可靠性。

## 4.Git本地仓库各种状态解析

### 工作区（Working Directory）

就是我们敲命令，改代码的当前目录，我们平时更新版本什么的，都是在这里完成的，可以理解成是在这里工作的。

### 版本库（Repository）

工作区有一个隐藏目录 `.git`，这个不算工作区，而是 Git 的版本库。可以认为有.git隐藏目录的目录算一个仓库，大仓库可以嵌套小仓库。

Git 的版本库里存了很多东西，其中最重要的就是称为 `stage`（或者叫 index）的暂存区，还有 Git 为我们自动创建[***<u>不是真正的创建，要先进行一次commit操作（进行一次提交操作），才会真正建立master分支。这是因为分支的指针要指向提交的,只有进行了提交，才有指针指向该分支，才算是真正的建立了分支，成为一个有效的对象。</u>***]的第一个分支 `master`，以及指向 `master`的一个指针叫 `HEAD`。

红色代表新增文件或者被修改文件，绿色代表位于暂存区的文件。

## 5.Git基本命令

### 创建版本库

```bash
#初始化仓库，在当前也就是敲git init命令的目录下多一个.git目录，提交的内容是指整个当前目录的所有内容
git init 
```

### 全局配置

```bash
#命令行输入user.name/user.email后，每一次Git提交都会使用这些信息，它们会写入到每一次提交中，用于标识身份,表明此次是由谁提交的。

git config --global user.name "username1"
git config --global user.email "xxx@163.com"

#查看配置信息

git config --list

#修改配置信息
git config --global --replace-all user.name "username2"
git config --global --replace-all user.email "xxx@qq.com"
```

### 查看库状态

```bash
#查看本地库状态，此时文件是untracked files，位于分支 master
git status
	尚无提交
	未跟踪的文件:
  	（使用 "git add <文件>..." 以包含要提交的内容）
        learngit/
	提交为空，但是存在尚未跟踪的文件（使用 "git add" 建立跟踪）
```

**未跟踪文件的产生主要有以下几种原因**：

1. 新添加的文件。
2. 生成的文件（如编译输出、临时文件等）。
3. 临时文件和日志文件。
4. 从远程仓库克隆后添加的文件。
5. 忽略文件的更改后重新生成的文件。

### 添加到本地暂存区

```shell
git add  <文件名1> <文件名2> ...
```

### 提交到本地

```shell
git commit -m "此次提交的信息"
```

### 修改提交信息

- 修改最近一次提交信息

  ```bash
  #这将打开默认的文本编辑器（通常是vi或nano），显示当前的提交信息
  git commit --amend
  ```

- 修改历史提交信息

  ```bash
  git rebase -i HEAD~N
  #其中 N 是想要回溯的提交数量。例如，如果想修改最近3次提交，可以使用HEAD~3 
  ```

### 比较差异

```shell
git diff readme.txt 


diff --git a/learngit/readme1.txt b/learngit/readme1.txt  #进行比较的是,a版本的readme1(即变动前)和b版本的readme1(即变动后).
index e69de29..b4de394 100644#表示两个版本的git哈希值(index区域的e69de29对象,与工作目录区域的b4de394对象进行比较),最后的六位数字是对象的模式(普通文件,644权限).
--- a/learngit/readme1.txt#表示进行比较的两个文件."-"表示变动前的版本,"+++"表示变动后的版本
+++ b/learngit/readme1.txt
@@ -0,0 +1 @@ -0,0 #表示旧版本的0到0行，+1表示新版本的0-1行
+11
```

### 撤销修改

1.撤销未暂存的修改（未add）

```shell
git checkout -- <被修改的文件名> 

git restore <file>
```

### 查看日志

```bash
#查看提交日志
git log

#查看历史git命令，寻找版本号
git reflog
```

### 版本回退

```shell
#回退到上一次commit，HEAD指向是当前活跃分支的最近一次提交。
git reset --hard HEAD^   
	[option]
		--hard #撤销最近一次提交，并丢弃所有未提交的更改
		--mixed #默认选项，撤销最近一次提交，取消暂存区的更改，但保留工作区的更改
		--soft  #撤销最近一次提交，但保留暂存区的更改

#根据版本号回退到版本
git reset --hard 1094a   
```

### 提交切换

切换版本通常是指切换到不同的提交（commit）、分支（branch）或标签（tag）

```shell
git checkout <commit-hash>

#注意：在这种情况下，Git 会进入“分离头指针”（detached HEAD）状态。在这种状态下，你的工作区将指向一个特定的提交，而不是一个分支。如果你在这个状态下进行提交，这些提交将不会关联到任何分支。如果你希望将这些提交关联到一个分支，可以创建一个新的分支：

git checkout -b <new-branch-name>
```

### 从分离头指针切换回分支

当前处于分离头指针状态（例如，通过  git checkout <commit-hash>   切换到了某个特定的提交），然后执行以下命令切换回一个分支：

```shell
git switch main
#或
git checkout main
```

## 7.Git分支命令

### 分支示意图

![GitBranch](C:\Users\z3254406361\Desktop\GitBranch.png)

### 查看分支

```bash
# 查看所有本地分支
git branch

# 查看所有远程分支
git branch -r

# 查看所有分支（本地和远程）
git branch -a
```

### 操作分支

```bash
#查看指定commit的id的文件的本次提交到哪些分支上了，即哪些分支包含了此处提交
git branch -a --contains cfc6773b07b6392144e124ee594aeaa6939e6bd7

#新增本地分支
git branch 分支名

#切换本地分支
git checkout 目的分支名

#删除本地分支
git branch -D 分支名

#合并目标分支到当前所在分支
git merge 目标分支名
```

### 合并分支冲突解决

1.修改冲突的文件
2.把修改好的冲突文件手动添加到暂存区
3.继续添加到版本库

## 8.Git远程管理

### 绑定本地和远程仓库

```shell
#克隆下来的仓库在本地的别名，省略默认为origin
git remote add <仓库别名> git@git:192.168.65.113/term/ModularizationTerminal.git
```

### 修改远程仓库绑定

```bash
git remote set-url <remote-name> <new-url>
```

### 使用git clone一键配置

在一个未初始化的目录中执行git clone命令时，Git会自动创建一个新的.git目录，并将克隆的仓库的内容复制到该目录中。此外,Git还会自动为克隆的仓库设置一个名为origin的远程仓库，指向克隆的源地址。
具体来说，git clone会执行以下操作：

1. 创建一个新的目录，如果没有指定目录名，Git会使用仓库的名称。
2. 在该目录中创建一个.git 目录，包含所有的版本控制信息。
3. 将远程仓库的所有文件和历史记录复制到本地。
4. 设置origin作为默认的远程仓库名称，指向克隆的源地址。
    因此，我们不需要在克隆之前手动执行git init，git clone会自动处理这些步骤。

### 克隆指定分支

```shell
#指定分支克隆，省略则为main或者master。
git clone -b <分支名> https://github.com/user/repo.git
```

### 查看远程仓库

```shell
git remote -v

origin  http://192.168.65.113/term/ModularizationTerminal.git (fetch)
origin  http://192.168.65.113/term/ModularizationTerminal.git (push)
zhangg  ssh://git@192.168.65.113:term/ModularizationTerminal.git (fetch)
zhangg  ssh://git@192.168.65.113:term/ModularizationTerminal.git (push)
```

### 从远程拉取

```shell
#从远程的仓库的指定分支下载代码到本地的当前分支上
git fetch <远程仓库名> <远程仓库分支>

git pull <远程仓库名> <远程分支名> == git fetch + git merge
```

### 推送到远程

推送步骤：提交本地修改——>pull合并远程代码到本地——>解决合并冲突——>push推送到远程

```bash
#如果远程仓库分支省略，则默认为与本地分支同名。
git push <远程仓库名> <本地分支名>[:远程分支名] 
```

## 9.Git定义别名

```shell
#全局配置文件，执行git --config命令的值都保存在此文件中
sudo vi ~/.gitconfig

[alias]
	ls = log
	st = status
	... ... 
	自定义名 = 命令名
```

## 10.设置不跟踪文件

### 使用   .gitignore   文件

1. 创建.gitignore（项目根目录下）

2. 添加忽略规则

   ```bash
   filename.txt  #忽略单个文件
   foldername/	  #忽略某个文件夹
   foldername/*  #忽略某个文件夹下的所有文件
   *.log
   *.tmp		 #忽略特定类型的文件
   foldername/filename.txt#忽略特定文件夹中的特定文件
   foldername/*.log #忽略特定文件夹中的特定类型文件
   ```

3. 保存并提交   .gitignore   文件

   ```bash
   git add .gitignore
   git commit -m "Add .gitignore file"
   ```

# Shell

## 1.shell脚本概述

shell脚本就是一些Linux命令的集合，执行shell脚本，就会自动执行脚本里全部的命令。

```sh
!#/bin/bash 
echo "hello world" #命令、选项与参数之间必须用空格隔开。
```

## 2.shell变量

### 变量分类

- 自定义变量
- 特殊变量：环境变量，只读变量，位置变量，预定义变量

### 变量特性

- 在shell编程中的变量通常使用全大写。
- 在变量前加$来引用变量的值
- Linux Shell/bash从右向左赋值
- 使用unset命令删除变量的赋值 

## 3.用户自定义变量

### 取值

- 双引号: 允许通过$符号引用其他变量值，弱引用，其中的变量引用会被替换为变量值
- 单引号: 禁止引用其他变量值，$视为普通字符，强引用，其中的变量引用不会被替换为变量值，而保持源字符串
- 反撇号: 命令替换，提取命令执行后的输出结果，``和$(…)作用相同

### 数组

- echo $a：默认显示数组a的第一个元素
- echo ${a[0]} ：显示数组中第一个元素，以此类推
- echo ${a[-1]}： 显示数组中最后一个元素
- echo ${a[*]} 和echo ${a[@]} ：显示数组中所有元素
- echo ${#a[@]} ：显示数组中元素的个数
- echo ${a[@]:起始元素id:元素个数]} ：显示数组中以起始元素为首的指定个数的元素（注意：这里起始元素id不能为负值
- unset a[n] 删除数组中的第n个元素
- unset a 删除a这个数组

## 4.特殊变量

### 只读变量

- 变量值不允许修改（重新赋值)的情况
- 无法使用 unset删除
- 最快方法重启
- readonly关键字

### 位置变量和预定义变量

- $0   与键入的命令行一样，包含脚本文件名
- $1,$2,……$9 ${10} 分别包含第一个到第十个命令行参数
- $#   包含命令行参数的个数，不包含$0
- $@  包含所有命令行参数：“$1,$2,……$9”
- $*  包含所有命令行参数，是一个整体：“$1,$2,……$9”
- $?  包含前一个命令的退出状态
- $$  包含正在执行进程的ID号

## 5.环境变量

### 环境变量介绍

​	在 Linux 系统中，环境变量（Environment Variables）是一些动态的值，它们会影响程序的运行时行为。环境变量通常用于存储配置信息、路径、用户偏好设置等，以便程序在运行时能够访问这些信息。

### 环境变量特点

- **全局性**：环境变量通常对所有进程都可用，包括用户**启动的程序和后台服务**。
- **动态性**：环境变量的值可以在运行时修改，修改后立即生效。
- **继承性**：当一个进程启动另一个进程时，子进程会继承父进程的环境变量

### 常见环境变量（都是大写）

#### 1.系统级

这些环境变量是在系统启动时由系统初始化脚本设置的，通常存储在 /etc/environment  文件中。这些变量对所有用户都有效。

1. **PATH** 
   • 用途：定义了系统在哪些目录中查找可执行文件。
   • 示例值：`/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin `
   • 作用：当在终端中运行一个命令时，系统会按照 PATH 中定义的目录顺序查找该命令的可执行文件。
2. **LANG 和 LC_*** 
   • 用途：定义系统的语言和区域设置。
   • 示例值：` en_US.UTF-8 ` 
   • 作用：影响程序的输出语言和日期、时间、数字等格式。
3. **SHELL** 
   • 用途：表示当前用户的默认Shell
   • 示例值：`/bin/bash `或 `/bin/tcsh `
   • 作用：当打开终端时，系统会启动这个Shell。
4. **PS1** 
   • 用途：定义 Bash Shell 的提示符。
   • 示例值：`\u@\h:\w\$ `
    • 作用：控制终端提示符的显示格式。
5. **EDITOR** 
   • 用途：定义默认的文本编辑器。
   • 示例值：  `vim   或   nano `
   • 作用：当运行需要编辑文本的命令（如   crontab -e  ）时，系统会启动这个编辑器。
6. **DISPLAY** 
   • 用途：定义显示设备。
   • 示例值：  :0 
   • 作用：用于图形界面程序，指定显示输出的设备。

#### 2.用户级

这些环境变量是在用户登录时由用户的 Shell 配置文件（ ~/.bashrc、 ~/.profile）加载的。

1. **HOME**
   • 用途：表示当前用户的主目录。
   • 示例值：`/home/username`或 `/Users/username`
   • 作用：许多程序会使用 HOME 目录来存储用户的配置文件和个人数据。
2. **USER** 
   • 用途：表示当前登录的用户名。
   • 示例值：`username`
   • 作用：程序可以通过这个变量获取当前用户的信息。

### 环境变量初始化

#### 登陆时初始化

当用户登录系统时，Shell 会按照一定的顺序加载配置文件，并设置环境变量。以下是常见的加载顺序（以 Bash Shell 为例）：

1. `/etc/profile`：系统级的初始化脚本，为所有用户设置通用的环境变量。
2. `~/.bash_profile`或`~/.bash_login`或` ~/.profile`：用户级的初始化脚本，为当前用户设置环境变量。
3. `~/.bashrc`：用户级的交互式Shell配置文件，通常包含用户常用的别名和函数。

#### 手动设置环境变量

-  **临时设置（仅对当前终端会话有效）**

  ```bash
  #例如：
  export MY_VAR="Hello, World!"
  echo $MY_VAR
  ```

- **永久设置（对所有终端会话有效）**

  ```bash
  #将变量添加到配置文件中，例如   ~/.bashrc或 ~/.bash_profile （取决于Shell 类型）。
  #编辑配置文件：
  nano ~/.bashrc
  #添加以下内容：
  export VAR_NAME=value
  #重新加载配置文件：
  source ~/.bashrc
  ```

### 环境变量继承性

环境变量的继承性当一个进程启动另一个进程时，子进程会继承父进程的环境变量。例如，当你在终端中启动一个程序时，该程序会继承终端的环境变量。

### export作用（临时有效）

1. **将变量导出为环境变量**
   	在 Bash Shell 中，变量默认是局部变量，仅在当前 Shell 环境中有效。使用 export 命令可以将局部变量导出为环境变量，使其对当前会话中的所有子进程可见。
   	这意味着，当在终端中设置了一个变量后，通过 export 导出该变量，所有后续启动的子进程（如脚本、命令等）都可以访问该变量。

   ```bash
   MY_VAR="Hello, World!"
   export MY_VAR
   echo $MY_VAR
   ```
2. **设置新的环境变量**
   可以直接通过export 命令设置新的环境变量，并为其赋值。

   ```bash
   export MY_VAR="Hello, World!"
   echo $MY_VAR
   ```
3. **修改现有环境变量**
   如果某个环境变量已经存在，可以通过 export 命令修改它的值。

   ```bash
   export PATH=$PATH:/home/username/bin
   echo $PATH
   ```

## 6.特殊符号

### 终止符

```bash
#语句终止符     ; 或者 回车换行
#语句块终止符	   ;;
```

### 占位符

在Bash中，如果直接比较一个变量和一个字符串，例如

```bash
if [ "$1" = "clean" ]; then
```

 当 $1 为空时，"$1" 会被扩展为空字符串，导致比较操作符两边的语法不完整。例如：

```bash
if [ = "clean" ]; then
```

这会导致语法错误，因为 [  命令会将 = 视为一个参数，而不是比较操作符。

- **任意字符** 
  X是一个常用的占位符字符，但它可以是任何字符。选择 x 只是因为它简单且不会与实际的变量值冲突。
- **确保字符串非空**
  通过添加 x ，可以确保比较操作符两边始终是有效的字符串，从而避免语法错误。



## 7.分支语句

注意：在算术运算，比大小，判断条件里的[ ]，**命令与参数之间需要加空格**。因为shell脚步的本质还是一些shell命令，shell命令对空格很敏感。

### 单路分支

then后面跟符合条件之后执行的程序，可以放在[]之后，用“;”分割。也可以换行写入，就不需要“;”了。

```shell
read -p "input a filename:" file
if test -e $file
then
	echo "$file exist"
elif [ -d $file];then
	echo "xxx"
else
	echo "$file is not exist"
fi

if [ -e $file ] #注意空格


即if[];then == if[]
				  then 
#即如果if[] then在同一行。用空格隔开，then会被当成if的参数，会报错，为了区分这是两条语句，需要用分号隔开
```

### 多路分支

```shell
#！/bin/bash
read -p "input yes/no:" val#read命令用于从标准输入（通常是键盘）读取用户输入，并将其存储到变量中

case $val in
	yes | Y)
		echo "input yes"
		;;
	no | n)
		echo "input no"
esac 
```

### 逻辑运算

```shell
if [$0 -a $1]....
-a #与
-o #或
```

###  算术运算

- 多层括号可以用`$($()+$())`实现
- 如果是 && || ,则要用if [ 表达式1 ] && [ 表达式2 ] 


### 循环分支

```shell
#1.
for a in {1,2,3,4,5}
do
	循环体  #执行一次有一个换行符
	if xxx then;
		break 
    else if xxx then;
		continue #立即结束本次循环，后面的没执行也不执行了
	fi
done

#2.
 for ((i=0;i<5;i=i+1))--两对括号间没有空格 
 
#3. 
 while ((i<5)) 

#4.
 while [ i -lt 5 ] 
```

### 其他语法

```shell
echo $? #获取函数返回值#

ret=`fun` #命令置换

#``和$()可以看作C语言中的define，但其赋值的内容仅限打印的内容，没有打印就没有值

#变量默认为全局变量，若fun里有一个变量，虽然默认为全局变量，但用命令置值时RET=`fun`时，fun外部还是不能用到该变量

$* :一个整体 ？？

$@:挨个打印？？
```



# AutoMake



1. 
