# `Linux`常用命令

## 1.文件管理

### 1.创建文件

```bash
vi a.txt 				  #默认打开a.txt，没有则创建再打开

touch <FILENAME>           #创建/打开文件并将文件最新修改时间更改为当前时间

touch File{1..5}.txt       #结合通配符，创建多个指定名称的空文件
```

### 2.查找文件

```bash
find 路径 条件 文件名
find [搜索路径] [选项] [正则表达式]
	[选项]
		-name	#根据文件名查找		eg:find ./ -name = *.txt
		-type	#根据文件类型查找       eg:find ./ -type = l
		-size	#根据文件大小查找       大于 n   -n：小于 n     n：等于 n
		-maxdepth n #搜索目录递归的深度为n，省略默认全盘搜索
		-exec…… {}\  #进一步处理搜索结果
#组合条件:查找当前目录及其子目录下所有属于用户 username 或大小大于 1MB 的文件
find . -type f \( -user username -o -size +1M \)


#全盘搜索系统中所有后缀为.mp4的文件，并删除所有查找到的文件
find / -name "*.mp4" -exec rm -rf {} \;
```

### 3.目录管理

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

### 4.查看文件类型

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

## 2.文档编辑

### 1.文本编辑器

```bash
vi example.txt		#在shell模式下,键入vi及需要编辑的文件名,即可进入vi. 

vi +5 example.txt	#如果需要在进入vi编辑界面后，将光标置于文件的第n行，则在vi命令后面加上“+n” 参数即可
```

### 2.显示文件内容

```bash
cat /etc/sysconfig/network	#cat <FILENAME>,将文件内容合并到标准输出，即输出文件内容
	[option]
		-n #查看行号信息
```

### 3.文件操作

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

### 4.批量编辑文本文件

`sed`命令来自英文词组stream editor的缩写，其功能是利用语法/脚本对文本文件进行批量的编辑操作。sed命令最初由贝尔实验室开发，后被众多Linux系统集成，能够通过正则表达式对文件进行批量编辑，让重复性的工作不再浪费时间。

## 3.系统管理

### 1.关机重启

```bash
sudo shutdown -h now/时间 [关机提示信息] #立即/定时关机
sudo shutdown -r now/时间 [重启提示信息] #立即/定时重启
sudo reboot                             #立即重启
```

### 2.系统主机信息

```bash
uname -n #显示主机名

uname -r #显示内容核版本

uname    #显示内核名，如Linux或者Unix

unmae -a #显示更加详细的信息

cat /etc/os-release  #查看操控操作系统发行版本
```

### 3.shell终端

```bash
clear             #清除终端所有打印的信息

history 		 #查询历史命令

!c       		 #执行上一个以c开头的命令

!g      		 #执行上一个以g开头的命令，常为gcc

table键  		#自动补齐
```

### 4.权限掩码

```bash
umask             #获取系统当前权限掩码，文件实际权限为创建文件时赋予的权限-权限掩码

umask 012 		# 设置新建文件权限默认为654（即666-umask）
```

### 5.切换用户

```bash
su [-l]  <用户名>	#切换到目标用户，加-l(环境PATH也会变)，看$前面是~还是username，如果是~表示切换了当前用户且当前环境也是该用户的，否则只是切换了用户并未切换环境，环境没变，有些有权限的操作不能使用。

su -c <命令>  <用户名>	#切换到目标用户执行一下该命令再切换回当前用户

su - linuxprobe			#完全变更至指定的用户身份，省略用户名则默认为root
exit                    #退出目标用户到自己的用户下
```

### 6.添加用户

```bash
sudo adduser <用户名>                  #添加用户
```

### 7.修改用户密码

```bash
sudo vi /etc/passwd                 #查看用户账户信息的文件
#passwd字段格式
#username:password:UID:GID:GECOS:home_directory:shell
#GECOS:用户信息字段，通常包含用户的真实姓名、电话号码、房间号码等信息。该字段主要用于描述用户，但具体格式和内容可能因系统而异。

passwd                               #修改当前用户密码

sudo passwd <用户名>                  #修改指定用户名密码
```

### 8.用户权限

```bash
vi      /etc/group                    #查看组

chmod <八进制> <FILENAME>              #修改文件权限

chmod [u | g | o | a(all)]  [+ | - | =]  [r | w | x] <FILENAME>
```

## 4.网络相关命令

### 1.网络测速

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

## 4.参考链接

https://www.kancloud.cn/digest/linux-world/145282



# Shell

## 1.shell脚本概述

shell脚本就是一些Linux命令的集合，执行shell脚本，就会自动执行脚本里全部的命令。

```sh
!#/bin/bash 
echo "hello world" #命令、选项与参数之间必须用空格隔开。
```

## 2.shell变量

### 1.变量分类

- 自定义变量
- 特殊变量：环境变量，只读变量，位置变量，预定义变量

### 2.变量特性

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

> [!CAUTION]
>
> 在算术运算，比大小，判断条件里的[ ]，**命令与参数之间需要加空格**。因为shell脚步的本质还是一些shell命令，shell命令对空格很敏感。因为Shell命令的空格是用来区分命令和选项的。

### 1.单路分支

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

# 注意事项

在Shell命令行中，空格是分隔命令参数的默认分隔符，所以当输入`cd 11 软件名`时bash会把它解析为两个参数:`11和软件名`，而不是一个完整的文件夹名`11 软件 `，解决办法就是用双引号括起成一个整体，或者给空格加反斜杠转义符。

# 工具网站

[Linux命令大全(手册) – 真正好用的Linux命令在线查询网站](https://www.linuxcool.com/)





