# 文件管理

## 1.创建文件

```bash
vi a.txt 				  #默认打开a.txt，没有则创建再打开

touch <FILENAME>           #创建/打开文件并将文件最新修改时间更改为当前时间

touch File{1..5}.txt       #结合通配符，创建多个指定名称的空文件
```

## 2.查找文件

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

## 3.目录管理

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

## 4.查看文件类型

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

# 文档编辑

## 1.文本编辑器

```bash
vi example.txt		#在shell模式下,键入vi及需要编辑的文件名,即可进入vi. 

vi +5 example.txt	#如果需要在进入vi编辑界面后，将光标置于文件的第n行，则在vi命令后面加上“+n” 参数即可
```

## 2.显示文件内容

```bash
cat /etc/sysconfig/network	#cat <FILENAME>,将文件内容合并到标准输出，即输出文件内容
	[option]
		-n #查看行号信息
```

## 3.文件操作

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

## 4.批量编辑文本文件

`sed`命令来自英文词组stream editor的缩写，其功能是利用语法/脚本对文本文件进行批量的编辑操作。sed命令最初由贝尔实验室开发，后被众多Linux系统集成，能够通过正则表达式对文件进行批量编辑，让重复性的工作不再浪费时间。

# 系统管理

## 1.关机重启

```bash
sudo shutdown -h now/时间 [关机提示信息] #立即/定时关机
sudo shutdown -r now/时间 [重启提示信息] #立即/定时重启
sudo reboot                             #立即重启
```

## 2.系统主机信息

```bash
uname -n #显示主机名

uname -r #显示内容核版本

uname    #显示内核名，如Linux或者Unix

unmae -a #显示更加详细的信息

cat /etc/os-release  #查看操控操作系统发行版本
```

## 3.shell终端

```bash
clear             #清除终端所有打印的信息

history 		 #查询历史命令

!c       		 #执行上一个以c开头的命令

!g      		 #执行上一个以g开头的命令，常为gcc

table键  		#自动补齐
```

## 4.权限掩码

```bash
umask             #获取系统当前权限掩码，文件实际权限为创建文件时赋予的权限-权限掩码

umask 012 		# 设置新建文件权限默认为654（即666-umask）
```

## 5.切换用户

```bash
su [-l]  <用户名>	#切换到目标用户，加-l(环境PATH也会变)，看$前面是~还是username，如果是~表示切换了当前用户且当前环境也是该用户的，否则只是切换了用户并未切换环境，环境没变，有些有权限的操作不能使用。

su -c <命令>  <用户名>	#切换到目标用户执行一下该命令再切换回当前用户

su - linuxprobe			#完全变更至指定的用户身份，省略用户名则默认为root
exit                    #退出目标用户到自己的用户下
```

## 6.添加用户

```bash
sudo adduser <用户名>                  #添加用户
```

## 7.修改用户密码

```bash
sudo vi /etc/passwd                 #查看用户账户信息的文件
#passwd字段格式
#username:password:UID:GID:GECOS:home_directory:shell
#GECOS:用户信息字段，通常包含用户的真实姓名、电话号码、房间号码等信息。该字段主要用于描述用户，但具体格式和内容可能因系统而异。

passwd                               #修改当前用户密码

sudo passwd <用户名>                  #修改指定用户名密码
```

## 8.用户权限

```bash
vi      /etc/group                    #查看组

chmod <八进制> <FILENAME>              #修改文件权限

chmod [u | g | o | a(all)]  [+ | - | =]  [r | w | x] <FILENAME>
```

# 网络相关

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

# 进程相关

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

# 输入输出

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
  

# 通配符

  ```bash
  *.c        #*匹配任意长度的字符串，即*可以代表任意长度的任意字串
  
  ?.c        #?匹配一个长度的字符串，即?可以代表任意一个字符
  
  [1,3,4].c  #匹配?.c其中的1，3，4，即1.c，3.c，4.c
  
  [1-5].c    #匹配?.c其中的1-5，即1，2，3，4，5
  
  [^1-5].c   #匹配?.c其中的除了1-5.c之外的。
  ```
# 管道命令

```bash
#管道前的命令一定要有输出结果才能传给管道后的命令

cat file | wc -l 

wc [option]
	[option]  -l  #统计有多少行内容
         	  -w  #多少个单词
              -c  #多少个字符
```

# 命令置换

```bash
a = `ls` #不让ls的结果输出到终端，而是把结果存到变量a中

echo $a

ls pwd   #系统以为ls列出名为pwd的文件

ls `pwd` #先执行pwd，再ls就是ls当前目录的内容了
```

  # 环境变量相关

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


# 删包 + 配置文件 + 不再需要的依赖
  sudo apt-get purge --autoremove <包名>

  purge = 删包 + 删配置文件（比 remove 干净）
  --autoremove = 顺带删掉没用的依赖

  如果想彻底扫干净残留：

  sudo apt-get purge --autoremove <包名>

  # 清缓存
  sudo apt-get clean

  # 查有没有漏的文件
  locate <包名>
  dpkg -L <包名> 2>/dev/null | xargs ls -la 2>/dev/null

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

# 注意事项

在Shell命令行中，空格是分隔命令参数的默认分隔符，所以当输入`cd 11 软件名`时bash会把它解析为两个参数:`11和软件名`，而不是一个完整的文件夹名`11 软件 `，解决办法就是用双引号括起成一个整体，或者给空格加反斜杠转义符。

# 工具网站

[Linux命令大全(手册) – 真正好用的Linux命令在线查询网站](https://www.linuxcool.com/)





