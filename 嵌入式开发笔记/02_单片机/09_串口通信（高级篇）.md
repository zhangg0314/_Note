# `minicom`串口通信工具

## 1.软件安装

```bash
sudo apt-get update
sudo apt-get install minicom
```



# 串口特殊字符

# Linux终端设置命令

## 1.命令介绍

`stty`（set terminal type）命令用来显示和更改终端行的设置。使用此命令，用户可以配置键盘输入的处理方式，控制输出显示的行为，或者设定特殊字符的功能。它对于串行通信和终端界面定制非常有用。

## 2.基本语法

语法格式如下：

```bash
stty [选项]... [设置]...
```

## 3.常用选项或参数说明

| 选项                | 描述                                                         |
| ------------------- | ------------------------------------------------------------ |
| `-a`, `--all`       | 显示所有当前设置的详细信息                                   |
| `-F` <串口设备路径> | 指定设备文件。比如指定要操作的串口设备（如 `/dev/ttyACM1`），默认操作当前终端（如 `stty -a` 查看当前终端参数），**对串口配置必须加 `-F` 指定设备**。 |
| `speed`             | 显示或设置波特率                                             |
| `echo`              | 开启输入回显                                                 |
| `-echo`             | 关闭输入回显                                                 |
| `cooked`            | 设置终端为正常模式                                           |
| `raw`               | 设置终端为原始模式                                           |
| `ispeed`            | 设置输入波特率                                               |
| `ospeed`            | 设置输出波特率                                               |
| `icanon`            | 开启规范模式                                                 |
| `-icanon`           | 关闭规范模式                                                 |

## 4.实例详解

### 1.显示所有设置

```bash
root@RK3576-Tronlong:~# stty -F /dev/ttyACM3 -a
speed 9600 baud;  #波特率（speed）：9600 baud
rows 0; columns 0; #没有设置行和列（rows 0; columns 0）
line = 0;
intr = ^C;  #Ctrl+C 用于中断程序
quit = ^\;  #Ctrl+\ 用于退出
erase = ^?; #退格键用于删除字符
kill = ^U;  #Ctrl+U 用于删除整行
eof = ^D;   #Ctrl+D 表示文件结束
eol = <undef>; eol2 = <undef>; swtch = <undef>; start = ^Q;
stop = ^S; susp = ^Z; rprnt = ^R; werase = ^W; lnext = ^V;
discard = ^O; min = 1; time = 0;
-parenb -parodd#无校验位
-cmspar 
cs8 #8 位数据位
hupcl 
-cstopb #1 位停止位
cread clocal 
-crtscts#关闭硬件流控（RTS/CTS 握手信号无效）
-ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr
icrnl #将回车转换为换行
ixon -ixoff -iuclc -ixany -imaxbel -iutf8
opost -olcuc -ocrnl onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0
bs0 vt0 ff0
isig 
icanon #启用规范模式（按行处理输入），有行缓冲，这意味着输入会被缓存，直到按下回车（\n）才会被程序读取，适合交互式输入，但不适合实时接收二进制数据或连续流数据。
iexten 
echo #启用输入回显
echoe echok -echonl -noflsh -xcase -tostop
-echoprt echoctl echoke -flusho -extproc
```

### 2.设置终端波特率

```bash
stty ispeed 9600 ospeed 9600
#此命令会将输入和输出的波特率都设置为9600。
```

### 3.禁用回显

```bash
stty -echo #输入字符时不会在终端显示，常用于密码输入。
```

### 4.启用回显

```bash
stty echo #恢复输入字符的显示。
```

### 6.切换到原始模式

```bash
stty raw#这将终端设置为原始模式，输入的字符会直接传递给程序，而不进行任何处理。
```

### 7.切换到正常模式

```bash
stty cooked
#这将终端设置为正常模式，输入会被行缓冲处理。
```

### 8.更改行结束符

在某些情况下，你可能需要更改行结束符（通常是回车键）。以下示例将行结束符从回车（CR）更改为换行符（LF）：

```bash
stty icrnl #这会把输入中的 CR 字符映射为 NL（换行）字符。
```

### 8.设置奇偶校验

在与某些硬件设备通信时需要设置奇偶校验，可以使用如下命令：

```bash
stty parenb parodd #这里，`parenb` 选项启用奇偶校验位的生成和检测，而 `parodd` 选项选择奇校验。
```

## 5.修改串口配置

```bash
stty -F /dev/ttyACM3 115200 -icanon -echo
# -表示取消，不带-表示启用
```

## 6.常用场景配置

如果这个串口用于连接传感器、PLC 等设备（非交互式场景），通常需要修改为：

```bash
# 关闭规范模式、回显和信号处理，禁用字符转换
stty -F /dev/ttyACM3 -icanon -echo -isig -icrnl -onlcr -opost
```

# Linux串口相关命令

查看设备是否被占用

```bash
lsof /dev/ttyUSB1  # 或 fuser /dev/ttyUSB1
```
