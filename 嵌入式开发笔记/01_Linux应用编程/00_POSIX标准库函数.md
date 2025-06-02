# 命令行解析

## 定义

传统的直接使用`argc` 和 `argv` 的需要手动遍历 `argv`，检查每个参数是否是选项，并处理选项的参数。而使用`getopt` 自动处理选项和参数，分离选项和非选项参数。`getopt` 支持短选项（如 `-h`）和长选项（如 `--help`）。`getopt` 提供了错误处理机制，当用户输入无效选项时，会自动打印错误信息。使用 `getopt` 可以大大简化命令行参数的解析逻辑，减少代码量。

```cpp
#include <unistd.h>
#include <getopt.h>
int getopt(int argc, char * const argv[], const char *optstring);
 
extern char *optarg;
extern int optind, opterr, optopt;


//参数说明：
	//getopt 参数说明：
		argc：通常由 main 函数直接传入，表示参数的数量
		argv：通常也由 main 函数直接传入，表示参数的字符串变量数组
		optstring：一个包含正确的参数选项字符串，用于参数的解析。例如 “abc:”，其中 -a，-b 就表示两个普通选项，-c 表示一个必须有参数的选项，因为它后面有一个冒号。
    //外部变量说明：
		optarg：如果某个选项有参数，这包含当前选项的参数字符串
		optind：argv 的当前索引值
		opterr：正常运行状态下为 0。非零时表示存在无效选项或者缺少选项参数，并输出其错误信息
		optopt：当发现无效选项字符时，即 getopt() 方法返回 ? 字符，optopt 中包含的就是发现的无效选项字符
 //返回值
            成功解析到选项时返回选项字符（如 'h'、'f' 等）。
		   解析完毕返回 -1。
 		   遇到无效选项或缺少参数返回 '?'。
```

## 示例代码

```c++
#include <stdio.h>
#include <unistd.h>
 
int main(int argc, char *argv[]) {
    int o;
    const char *optstring = "abc:"; // 有三个选项-abc，其中c选项后有冒号，所以后面必须有参数
    while ((o = getopt(argc, argv, optstring)) != -1) {
        switch (o) {
            case 'a':
                printf("opt is a, oprarg is: %s\n", optarg);
                break;
            case 'b':
                printf("opt is b, oprarg is: %s\n", optarg);
                break;
            case 'c':
                printf("opt is c, oprarg is: %s\n", optarg);
                break;
            case '?':
                printf("error optopt: %c\n", optopt);
                printf("error opterr: %d\n", opterr);
                break;
        }
    }
    return 0;
}
```

# termios API

## 1.概念介绍

串口的应用编程可以通过`ioctl()`对串口进行配置，调用` read()`读取串口的数据、调用` write()`向串口写入数据。但是通常不这么做！因为 Linux 为上层用户做了一层封装，将这些` ioctl(`)操作封装成了一套标准的 API。可以直接使用这套标准 API 进行串口应用编程。**注：这一套接口并不是针对串口开发的，而是针对所有的终端设备。**

## 2.struct termios结构体

### 1.结构体成员

```c

注：对于这些变量尽量不要直接对其初始化，而要将其通过“按位与”、“按位或” 等操作添加标志或清除某个标志。

注：不同的终端设备，本身硬件上就存在很大的区别，所以配置参数不是对所有终端设备都是有效的。struct termios
{
    tcflag_t c_iflag; /* input mode flags */
    tcflag_t c_oflag; /* output mode flags */
    tcflag_t c_cflag; /* control mode flags */
    tcflag_t c_lflag; /* local mode flags */
    cc_t c_line; /* line discipline */
    cc_t c_cc[NCCS]; /* control characters */
    speed_t c_ispeed; /* input speed */
    speed_t c_ospeed; /* output speed */
};
//注：对于这些变量尽量不要直接对其初始化，而要将其通过“按位与”、“按位或” 等操作添加标志或清除某个标志。
//注：不同的终端设备，本身硬件上就存在很大的区别，所以配置参数不是对所有终端设备都是有效的。
```

### 2.输入模式标志

**c_iflag的取值**

| 宏     | 说明                                                |
| ------ | --------------------------------------------------- |
| IGNBRK | 忽略输入终止条件                                    |
| BRKINT | 当检测到输入终止条件时发送 SIGINT 信号              |
| IGNPAR | 忽略帧错误和奇偶校验错误                            |
| PARMRK | 对奇偶校验错误做出标记                              |
| INPCK  | 对接收到的数据执行奇偶校验                          |
| ISTRIP | 将所有接收到的数据裁剪为 7 比特位、也就是去除第八位 |
| INLCR  | 将接收到的 NL（换行符）转换为 CR（回车符）          |
| IGNCR  | 忽略接收到的 CR（回车符）                           |
| ICRNL  | 将接收到的 CR（回车符）转换为 NL（换行符）          |
| IUCLC  | 将接收到的大写字符映射为小写字符                    |
| IXON   | 启动输出软件流控                                    |
| IXOFF  | 启动输入软件流控                                    |
| IXANY  | 允许任意字节启动流控                                |

### 3.输出模式标志

**c_oflag**

| 宏     | 说明                                                     |
| ------ | -------------------------------------------------------- |
| OPOST  | 启用输出处理功能，如果不设置该标志则下面其他标志都被忽略 |
| OLCUC  | 将输出字符中的大写字符转换成小写字符                     |
| ONLCR  | 将输出中的换行符（NL '\n'）转换成回车符（CR '\r'）       |
| OCRNL  | 将输出中的回车符（CR '\r'）转换成换行符（NL '\n'）       |
| ONOCR  | 在第 0 列不输出回车符（CR）                              |
| ONLRET | 不输出回车符                                             |
| OFILL  | 发送填充字符以提供延时                                   |
| OFDEL  | 如果设置该标志，则表示填充字符为 DEL 字符，否则为 NULL   |

### 4.字符

**c_cflag**
控制模式控制终端设备的硬件特性（串口波特率、数据位、校验位、停止位等）。

| CBAUD    | 波特率的位掩码 |
| -------- | -------------- |
| B0       | 波特率为 0     |
| B300     | 300 波特率     |
| B1200    | 1200 波特率    |
| B1800    | 1800 波特率    |
| B2400    | 2400 波特率    |
| B4800    | 4800 波特率    |
| B9600    | 9600 波特率    |
| B19200   | 19200 波特率   |
| B38400   | 38400 波特率   |
| B57600   | 57600 波特率   |
| B115200  | 115200 波特率  |
| B230400  | 230400 波特率  |
| B460800  | 460800 波特率  |
| B500000  | 500000 波特率  |
| B576000  | 576000 波特率  |
| B921600  | 921600 波特率  |
| B1000000 | 1000000 波特率 |
| B1152000 | 1152000 波特率 |
| B1500000 | 1500000 波特率 |
| B2000000 | 2000000 波特率 |
| B2500000 | 2500000 波特率 |
| B3000000 | 3000000 波特率 |

注：在 Linux 系统下， 使用 `CBAUD `位掩码所选择的位来指定串口波特率；在其它一些系统中，可能使用`c_ispeed `成员变量和 `c_ospeed `成员变量来指定串口的波特率。

| CSIZE   | 数据位的位掩码                                 |
| ------- | ---------------------------------------------- |
| CS5     | 5 个数据位                                     |
| CS6     | 6 个数据位                                     |
| CS7     | 7 个数据位                                     |
| CS8     | 8 个数据位                                     |
| CSTOPB  | 2 个停止位，如果不设置该标志则默认是一个停止位 |
| CREAD   | 接收使能                                       |
| PARENB  | 使能奇偶校验                                   |
| PARODD  | 使用奇校验、而不是偶校验                       |
| HUPCL   | 关闭时挂断调制解调器                           |
| CLOCAL  | 忽略调制解调器控制线                           |
| CRTSCTS | 使能硬件流控                                   |

### 5.本地工作模式

**c_lflag**本地模式用于控制终端的本地数据处理和工作模式。

|         |                                                              |
| ------- | ------------------------------------------------------------ |
| ISIG    | 若收到信号字符（INTR、 QUIT 等），则会产生相应的信号         |
| ICANON  | 启用规范模式                                                 |
| ECHO    | 启用输入字符的本地回显功能。                                 |
| ECHOE   | 若设置 ICANON，则允许退格操作                                |
| ECHOK   | 若设置 ICANON，则 KILL 字符会删除当前行                      |
| ECHONL  | 若设置 ICANON，则允许回显换行符                              |
| ECHOCTL | 若设置 ECHO，则控制字符（制表符、换行符等）会显示成“^X”，其中 X 的 ASCII 码等于给相应控制字符的 ASCII 码加上 0x40。例如，退格字符（0x08）会显示为“^H”（'H'的 ASCII 码为 0x48） |
| ECHOPRT | 若设置 ICANON 和 IECHO，则删除字符（退格符等）和被删除的字符都会被显示 |
| ECHOKE  | 若设置 ICANON，则允许回显在 ECHOE 和 ECHOPRT 中设定的 KILL字符 |
| NOFLSH  | 在通常情况下，当接收到 INTR、 QUIT 和 SUSP 控制字符时，会清空输入和输出队列。如果设置该标志，**则所有的队列不会被清空** |
| TOSTOP  | 若一个后台进程试图向它的控制终端进行写操作，则系统向该后台进程的进程组发送 SIGTTOU 信号。该信号通常终止进程的执行 |
| IEXTEN  | 启用输入处理功能                                             |

### 6.字符组合

**c_cc**特殊控制字符是一些字符组合，如 Ctrl+C、 Ctrl+Z 等， 当用户键入这样的组合键，终端会采取特殊处理方式。

|        |                                                              |
| ------ | ------------------------------------------------------------ |
| VEOF   | 文件结尾符 EOF，对应键为 Ctrl+D； 该字符使终端驱动程序将输入行中的全部字符传递给正在读取输入的应用程序。如果文件结尾符是该行的第一个字符，则用户程序中的 read 返回 0，表示文件结束。 |
| VEOL   | 附加行结尾符 EOL，对应键为 Carriage return（CR） ； 作用类似于行结束符。 |
| VEOL2  | 第二行结尾符 EOL2，对应键为 Line feed（LF） 。               |
| VERASE | 删除操作符 ERASE，对应键为 Backspace（BS） ； 该字符使终端驱动程序删除输入行中的最后一个字符； |
| VINTR  | 中断控制字符 INTR，对应键为 Ctrl+C； 该字符使终端驱动程序向与终端相连的进程发送SIGINT 信号； |
| VKILL  | 删除行符 KILL，对应键为 Ctrl+U， 该字符使终端驱动程序删除整个输入行； |
| VMIN   | 在非规范模式下，指定最少读取的字符数 MIN；                   |
| VQUIT  | 退出操作符 QUIT，对应键为 Ctrl+Z； 该字符使终端驱动程序向与终端相连的进程发送SIGQUIT 信号。 |
| VSTART | 开始字符 START，对应键为 Ctrl+Q； 重新启动被 STOP 暂停的输出。 |
| VSTOP  | 停止字符 STOP，对应键为 Ctrl+S； 字符作用“截流”，即阻止向终端的进一步输出。用于支持 XON/XOFF 流控。 |
| VSUSP  | 挂起字符 SUSP，对应键为 Ctrl+Z； 该字符使终端驱动程序向与终端相连的进程发送SIGSUSP 信号，用于挂起当前应用程序。 |
| VTIME  | 非规范模式下， 指定读取的每个字符之间的超时时间（以分秒为单位） TIME。 |

## 3.函数

### 1.cfmakeraw()函数

```c
//将终端配置为原始模式。将终端设置为一种简单的、原始的模式，这种模式下终端不会进行一些复杂的处理，如回显、行编辑等

#include <termios.h>
#include <unistd.h>
void cfmakeraw(struct termios *termios_p);
```

### 2.cfsetispeed()函数

```c
//设置输入波特率。
//注：设置波特率有专门的函数，用户不能直接通过位掩码来操作。

#include <termios.h>
#include <unistd.h>

int cfsetispeed(struct termios *termios_p, speed_t speed);
```

### 3.cfsetospeed()函数

```c
//设置输出波特率。

//注：设置波特率有专门的函数，用户不能直接通过位掩码来操作。

#include <termios.h>
#include <unistd.h>

int cfsetospeed(struct termios *termios_p, speed_t speed);
```

### 4.cfsetspeed()函数

```c
//设置波特率。
//注：设置波特率有专门的函数，用户不能直接通过位掩码来操作。

#include <termios.h>
#include <unistd.h>

int cfsetspeed(struct termios *termios_p, speed_t speed);
```

### 5.tcdrain()函数

```c
//调用 tcdrain()函数后会使得应用程序阻塞， 直到串口输出缓冲区中的数据全部发送完毕为止！

#include <termios.h>
#include <unistd.h>

int tcdrain(int fd);	
	//参数 fd:文件描述符。
	//返回值：调用成功时返回 0；失败将返回-1，并设置 errno。 
```

### 6.tcflush()函数

```c
//调用该函数会清空输入/输出缓冲区中的数据。

#include <termios.h>
#include <unistd.h>

int tcflush(int fd, int queue_selector);
	//参数 fd:文件描述符。
	//参数 queue_selector：
			TCIFLUSH	对接收到而未被读取的数据进行清空处理
			TCOFLUSH	对尚未传输成功的输出数据进行清空处理
			TCIOFLUSH	对尚未处理的输入/输出数据进行清空处理
	//返回值：调用成功时返回 0；失败将返回-1，并设置 errno
```

### 7.tcflow()函数

```c
//调用 tcflow()函数会暂停数据传输或接收工作。

#include <termios.h>
#include <unistd.h>

int tcflow(int fd, int action);
 //参数 fd:文件描述符
 //参数 action:
		TCOOFF	暂停数据输出（输出传输）
		TCOON	重新启动暂停的输出
		TCIOFF	发送 STOP 字符，停止终端设备向系统发送数据
		TCION	发送一个 START 字符，启动终端设备向系统发送数据；
//返回值：调用成功时返回 0；失败将返回-1，并设置 errno。
```

### 8.tcgetattr()函数

```c
//获取到终端当前的配置参数。

#include <termios.h>
#include <unistd.h>

int tcgetattr(int fd, struct termios *termios_p);
	//参数 fd:文件描述符
	//参数 termios_p：保存获取的配置参数。
	//返回值：调用成功时返回 0；失败将返回-1，并设置 errno。
```

### 9.tcsetattr()函数

```c
//将配置参数写入到终端设备，使其生效。

#include <termios.h>
#include <unistd.h>

int tcsetattr(int fd, int optional_actions,const struct termios *termios_p);

	//参数 fd:文件描述符
	//参数 optional_actions:指定更改何时生效
			TCSANOW		配置立即生效
			TCSADRAIN	配置在所有写入 fd 的输出都传输完毕之后生效
			TCSAFLUSH	所有已接收但未读取的输入都将在配置生效之前被丢弃
	// termios_p:将 struct termios 对象中的配置参数写入到终端设备中。
	//值：调用成功时返回 0；失败将返回-1，并设置 errno。
```

## 4.串口应用编程流程

1. 调用`open()`打开串口设备文件

   **注**：调用 `open()`函数时，使用` O_NOCTTY `标志，该标志用于告知系统它不会成为进程的控制终端。

2. 调用`cfmakeraw()`将终端配置为原始模式

3. 接收使能

4. 调用`tcflush()`刷新缓冲区

5. 设置输入输出波特率

6. 设置数据位

7. 设置停止位

8. 设置奇偶校验位

9. 设置 MIN 和 TIME 的值

10. 调用`tcsetattr()`更新配置

11. 调用read()/write()读写数据

## 5.示例(A9安防监控项目)

```c
int set_com_config(int fd, int baud_rate, int data_bits, char parity, int stop_bits)
{
	struct termios new_cfg, old_cfg;
	soeed_t speed;
	/*保存原有串口配置*/
	if (tcgetattr(fd, &old_cfg) != 0){
		perror("tcgetattr");
		return -1;
	}
	new_cfg =old_cfg;
	/*配置为原始模式*/
	cfmakeraw(&new_cfg);
	new_cfg.c_cflag &= ~CSIZE;//把size位清0,CSIZE 是 0x001F（即 0001 1111），~CSIZE 是 CSIZE 的反码，即 1110 0000。通过 &= 操作，可以将 c_cflag 中的数据位数相关的位清零。
	/*设置波特率*/
	switch (baud_rate)
	{
		case 2400:{
					  speed = B2400;
					  break; 
				  }
		case 4800:{
					  speed = B4800;
					  break;
				  }
		case 9600:{
					  speed = B9600;
					  break;
				  }
		case 19200:{
					   speed = B19200;
					   break;
				   }
		case 38400:{
					   speed = B38400;
					   break;
				   }

		default:
		case 115000:{
						speed = B115200;
						break;
					}
	}

	cfsetispeed(&new_cfg, speed);
	cfsetospeed(&new_cfg, speed);

	/*设置数据位*/
	switch (data_bits)
	{
		case 7:{
				   new_cfg.c_cflag |= CS7;
				   break;
			   }   
		default:	
		case 8:{
				   new_cfg.c_cflag |= CS8;
				   break;
			   }
	}

	/*设置奇偶校验位*/
	switch (parity)
	{
		default:
		case 'n':
		case 'N':{
					 new_cfg.c_cflag &= ~PARENB;
					 new_cfg.c_iflag &= ~INPCK;
					 break;
				 }
		case 'o':
		case 'O':{
					 new_cfg.c_cflag |= (PARODD |PARENB);
					 new_cfg.c_iflag |= INPCK;
					 break;
				 }
		case 'e':
		case 'E':{
					 new_cfg.c_cflag |= PARENB;
					 new_cfg.c_cflag &= ~PARODD;
					 new_cfg.c_iflag |= INPCK;
					 break;
				 }
		case 's':
		case 'S':{
					 new_cfg.c_cflag &= ~PARENB;
					 new_cfg.c_cflag &= ~CSTOPB;
					 break;
				 }
	}

	/*设置停止位*/
	switch (stop_bits)
	{
		default:
		case 1:{
				   new_cfg.c_cflag &= ~CSTOPB;
				   break;
			   }   	
		case 2:{
				   new_cfg.c_cflag |= CSTOPB;
				   break;
			   }
	}

	/*设置等待时间和最小接收字符*/
	new_cfg.c_cc[VTIME] = 0;
	new_cfg.c_cc[VMIN] = 1;
	tcflush(fd, TCIFLUSH);//刷新了接受和发送缓冲区
	if ((tcsetattr(fd, TCSANOW, &new_cfg)) != 0)//通过new_cfg设置终端参数
	{
		perror("tcsetattr");
		return -1;
	}

	return 0;
} 
int open_port(char *com_port)
{
    int fd;
	/*打开串口*/
	fd = open(com_port, O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd < 0){
		perror("open serial port");
		return -1;
	}
    /*恢复串口阻塞状态*/
    if(fcntl(fd, F_SETFL, 0) < 0){
		perror("fcntl F_SETFL\n");
	} 
	/*判断是否为终端设备*/
	if (isatty(fd) == 0){
		perror("This is not a terminal device");
	} 
	return fd;
}

/*--------------------CH340Ƥ׃---------------------------*/
void USB_UART_Config(char* path, int baud_rate)
{
	int fd;
	fd = open_port(path);
	if(fd < 0){
		printf("open %s failed\n",path);
		return ;
	}
	if (set_com_config(fd, baud_rate, 8, 'N', 1) < 0)
	{
		perror("set_com_config");
		return ;
	}
	close(fd);
	return ;
}
```


   
