# 		——网络基础概念——  

# 		1.什么是互联网组成

## 1.计算机网络的定义

计算机网络的精确定义并未统一，下面是用的比较多的两个定义：

- 以功能完善的网络软件及通信协议实现资源共享和信息传递的系统
- 以传输信息为基本目的，用通信线路和通信设备将多个计算机连接起来的计算机系统的集合

## 2.网络的分类

### 1.按作用范围分

广域网、城域网、局域网、个人区域网（如个人热点）

### 2.按网络使用者分

公用网，专用网（如军队使用的网络）

## 3.网络的组成

**计算机网络的组成：**

​	由若干节点和连接这些节点的链路组成，这些节点可以是计算机，集线器，交换机或路由器等。

**互联网络的组成：**

由多个网络通过路由器连接在一起，构成一个覆盖范围更大的计算机网络。也就是网络的网络。

## 4.互联网介绍

- 全球最大的，开放的，由众多网络相互连接而成的特定互连网络
- 采用TCP/IP协议族作为通信规则
- 前身是美国得ARPAnet（阿帕网）

## 5.互联网的组成

- **核心部分**，由网络路由器组成，路由器虽然也是计算机，但在网络中不能成为主机
- **边缘部分**，由主机（又叫端系统）和集线器组成

# 2.协议与层次划分

## 1.协议

网络协议，简称协议，是为进行网络中的数据交互而建立得规则标准或约定。

**协议的三个组成要素**：

- 语法：数据与控制信息的结构或格式，如编码格式。
- 语义：需要发出何种控制信息，完成何种动作以及做出何种响应，如帧结构。
- 同步：事件实现顺序的详细说明，如帧请求，帧应当。

## 2.划分层次

举例：两台主机通过网络传送文件。

![image-20241004111424468](figure\image-20241004111424468.png)

1. 将文件传送模块作为最高的一层
2. 再设计一个通信服务模块层
3. 网络接入模块：负责做与网络接口细节有关的工作，并为上层提供接入和通信服务。

## 3.各层完成的主要功能

- **差错控制**：使相应层次对等方的通信更加可靠
- **流量控制**：发送端的发送速率必须使接收端来得及接收，不要太快
- **分段和重装**：发送端将要发送的数据块划分为更小的单位，在接收端将其还原。
- **复用和分用**：发送端几个高层会话复用一条低层的连接，在接收端再进行分用。
- **连接的建立和连接**：交互数据前先建立一条逻辑连接，数据传输结束后释放连接。

## 4.计算机网络体系结构

- 网络体系结构是计算机网络的各层及其协议的集合，就是这个计算机网络及其构件所应完成的功能的精确定义（不涉及实现）。
- 实现是遵循这种体系结构的前提下，用任何硬件或软件完成这些功能的问题。
- 体系结构是抽象的，而实现则是具体的，是正真运行的计算机硬件和软件。

# 		3.OSI七层体系结构（理论模型）

（赢表回，船往叔屋）

OSI，即开放互联参考模型。**虽然市场上未真正实现，但具有很大的参考价值，如zigbee的zstack协议栈就是参考该七层模型的**。

## 1.高层：负责主机之间的数据处理

- **应用层**
  网络服务与最终应用的一个接口，为用户应用程序提供服务，定义了应用程序之间通信的协议和规则。
- **表示层**
  数据的表示（如文字，视频表示方法）
  安全
  解压缩：压缩数据，提高底层传送效率。负责数据的压缩，安全加密，格式转换等功能
- **会话层**
  建立，管理，中止会话，负责主机间的数据传输
  会话：本地应用之间如何进行通信，比如微信支付时，微信与淘宝的通信。负责建立、管理和终止会话，允许不同机器上的用户之间建立和管理会话。

## 2.低层：负责网络之间的数据传输

- **传输层**
  定义传输数据的协议端口号，以及流控，差错校验,校验的是端口号以及复用和分用（端口号表明不同应用）。负责数据报的传输，向下透明，向上提高服务，负责的是端到端的传输服务

- **网络层**
  进行网络地址寻址，差错校验，校验的是地址
  主要负责IP数据报的传输，有路由选择，地址映射等功能，负责的是主机到主机的传输服务

- **数据链路层**

  建立逻辑连接，进行**硬件地址**寻址，差错校验（进行数据的校验）
  主要负责数据帧的传输，有差错控制，流量控制功能

- **物理层**

  建立，维护，断开物理连接，传输比特流。
  主要负责传输比特流，实现设备间的物理连接

# 		4.TCP/IP及五层体系结构（实际应用模型）

## 1.TCP/IP四层协议体系结构

![image-20241004115313973](figure\image-20241004115313973.png)

以 IP 为内核，只考虑 IP 网络层以上的几层，不考虑 IP 下面的网络接口层具体的细节。所有才叫TCP/IP协议。设计理念：网络核心越简单越好。
![image-20241004120252185](figure\image-20241004120252185.png)

## 2.TCP/IP体系结构的另一种表示

现在互联网使用的 TCP/IP 体系结构已经发生演变，即某些应用程序可以直接使用IP层，或甚至直接使用最下面的网络接口层，比如Ping命令。![image-20241004115921120](figure\image-20241004115921120.png)

## 3.五层体系结构

真正现实中需要考虑到的。

- **应用层**
  
  任务：通过应用进程间的交互来形成特定网络应用。
  协议：定义的是应用进程间的通信和交互的规则。把应用交互的数据单元是**数据报**。
  
  例如：tftp，http，dns，smtp，telnet
  
- **传输层**
  任务：负责向两台主机中进程之间的通信提供**通用**的数据传输服务服务，不提供数据协议，只考虑发送接收。
  具有复用和分用的功能
  tcp（可靠，面向连接，**报文段**，保证可靠性）,udp（不可靠，无连接，**用户数据报**，保证时效性）
  
- **网络层**
  **IP数据报，**
  功能：为分组交换网上的不同主机提供通信服务。具体
  任务：路由选择和转发
  
  协议：ip协议+路由选择协议
  ip,icmp,ospf,rip
  
- **数据链路层**
  **帧**，
  功能：实现两个相邻节点之间的可靠传输，如有差错，就简单的丢弃出错的帧。如果需要更正错误，就要靠可靠传输协议。
  arp,mtu,
  
- **物理层**
  **比特流**，功能：实现比特0，1的传输。


# 5.IP地址

## 1.表示方法

**实质**:32位二进制数

**表示**:点分十进制数

2级结构：ip地址 :: = {<网络号>,<主机号>}

3级结构（划分子网）：IP地址 ::={<网络号>，<子网号>，<主机号>}

## 2.分类IP

任意一个IP地址我们都可以迅速的得出类别，并计算得出网络号。

- A类：0开头，8位网络号，0.x.x.x~127.x.x.x

- B类：10开头，16位网络号 128.x.x.x~191.x.x.x

- C类：110开头，24位网络号192.x.x.x~223.x.x.x

- D类：1110开头，主要用于多播地址224.x.x.x~

- E类：1111开头，保留240.x.x.x~

## 3.分类IP优缺点

1. **优点**：

   管理简单，使用方便，转发分组迅速，划分子网，灵活地使用。

2. **缺点**：
   1、设计上不合理，地址块太大，浪费了大量的地址。
   2、即使采用划分子网方法解决，也不能解决 IP 地址枯竭得问题。
   
3. **划分子网仍然存在的缺点**：
   C类可指派主机数才255-2，很少企业组织使用，所以C类地址被大量浪费。
   解决：无分类编址

## 4.无分类编址 CIDR 

### 1.CIDR

- **CIDR（classless inter-domain routing）即无分类域间路由选择。**
- IP地址 :: = {<网络前缀>，<主机号>}，前缀n不固定，取值范围0~32
- 记法：斜线记法，a.b.c.d/n，如:128.14.35.7/20表示前20位为网络号


### 2.子网掩码

**子网掩码**：
	网络位全为1，主机位全为0，由一连串1和接着的一连串0组成，而1的个数就是网络前缀的长度。

**目的**：
	让机器从 IP 地址迅速算出网络地址。直接进行按位与运输就能得出某个IP地址所属的网络号。

## 5.特殊IP

- **多归属主机**
  当一个主机通过多个个网卡同时连接到多个个网络时，会同时有多个 ip 地址，也就同时处于多个网络之中，这样的主机叫做多归属主机。
- **IP地址全0**
  表示在本网络上的本主机，只能做源地址
- **网络号全0,主机号为xx**
  表示本网络上主机号为xx的主机，只能做源地址
- **全1的IP地址**
  因为路由器不转发，所以只能在本网络广播，只能做目的地址
- **主机号全1，网络号为Y**
  向网络Y广播，只能做目的地址
- **127.xxxx**
  本地软件环回测试，可以做目的和源地址
- **总结**![image-20241004140827810](figure\image-20241004140827810.png)		

## 6.IPV6的表示方法

- **大小**：占128位
- **记法**：冒号16进制，每隔两个字节加一个冒号，
- **0压缩**（<u>只能有一处地方压缩</u>）：一连串的0可以用两个冒号表示。

# 6.端口号

## 1.进程间的通信

![image-20241004143837204](figure\image-20241004143837204.png)

## 2.协议端口号

### 传输协议

​	网络层主要负责的是主机之间通信，而运输层负责的应用层的进程之间通信。运输层的协议是全双工的，有TCP（不提供广播多播方式），UDP协议。

### 传输协议数据单元（TPDU）

​	TCP的TPDU叫TCP报文段，UDP的TPDI叫UDP报文或用户数据段。

![image-20241004145224297](figure\image-20241004145224297.png)

![image-20240613224933519](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240613224933519.png)

- 0-1023：服务器熟知端口，全球通用，IANA负责分配
- 1024-49151：服务器登记端口
- 49152-65535：客户端使用端口，暂用动态的，一般要回收

## 3.Linux的BSD端口

BSD也是一个标准

![**image-20240613225400213**](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240613225400213.png)

![image-20241004150126753](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20241004150126753.png)

建议自己写的程序绑定端口号为5001~32768

# ——Linux网络编程——

# 1.字节序

## 1.介绍

- 字节序是指多字节数据在计算机内存中存储或者网络传输时个字节存储的顺序。

- 大端字节序，小段字节序

- 一般主机使用小端（方便进行进位加减），而网络中用大端（方便读取和位运算）


## 2.字节序转换

- **本机转网络**

  ```c
  uint32_t htonl(uint32_t hostlong);
  
  uint16_t htons(uint16_t hostshort);
  ```

- 网络转本机

  ```c
  uint32_t ntohl(uint32_t hostlong);
  
  uint16_t ntohs(uint16_t hostshort);
  ```

# 		2.socket套接字

## 1.套接字概念及表示

![image-20241004162119409](figure\image-20241004162119409.png)

**三元组**：【IP地址，端口号，协议】

- **IP：**标识计算机

- **端口号**：标识计算机中的进程
- **协议**:指定数据传输方式,主要指tcp和udp

应用层主要位于用户空间的应用程序的编写，传输层及传输层以下的网络层，数据链路层等主要由操作系统的内核实现，我们只需要指定套接字的协议类型是tcp还是udp以及ip地址和端口号就行，所有就利用到了socket编程。

## 8.2.socket常用API介绍

1. **创建套接字**

   ```c
   int socket(int domain,int type,int protocol);
   	//返回值：成功返回文件描述符，失败返回-1
   	//参数说明：
   	domain：通信地址族，如IPV4（AF_INET）还是IPV6（AF_INET6），还是本地通信
   	type：套接字类型，取值为宏
       protocol：协议类型，
             由于流式套接字，数据报套接字分别只对应TCP,UDP，故取值为0即可，
             若类型为原始套接字，对应IP,ICMP等多种协议，则本参数需要指定特殊值
   ```

2. **绑定通信结构体**

   ```c
   int bind(int sockfd,const struct sockaddr *addr,socklen_t addrlen);
   	//返回值：成功返回0，失败返回-1
   	//参数说明：
   	1.sockfd:socket函数生成的套接字，即socket函数的返回值
       2.addr：通用的通信结构体
               struct sockaddr{
                            sa_family_t sa_family;
                            char sa_data[14];
                }//通用地址族结构体
                通信地址族       通信地址族结构体
                AF_INET       struct sockaddr_in{
                       				sa_famiy_t sin_family;//通信地址族
                       				in_port_t sin_port;//网络字节序的端口号
                       				struct in_addr sin_addr;//IP地址结构体
                       					//struct in_addr{
                       						//uint32_t s_addr;//网络字节序的IP地址
                   						//}
                   			  }
   						
                   AF_INET6      struct sockaddr_in6{...}
                    ...				...
                    ...				...
         3.addrlen：结构体长度,sizeof(addr)
   ```

3. **监听套接字**

   ```c
   int listen(int sockfd,int backlog);
   	//sockfd:socket函数的返回值
   	//backlog:监听数量，即可以同时连接的客户端数量，即维护接收客户端队列的长度
   ```

4. **接收连接**

   ```c
   int accept(int sockfd,struct sockaddr *addr,socklen_t *addrlen)//阻塞函数
       //功能：服务器接收客户端的连接请求
       //返回值：返回新的套接字（也是文件描述符），newsockfd（最终要读写的文件描述符），由用户端的ip和端口号组成。
       //参数说明：
       addr:传出参数，由函数负责i填充客户端ip及端口号到addr中。
       addrlen:调用方必须初始化它为包含指向的结构的大小（以字节为单位）ADDR;返回时，它将包含对等地址的实际大小.即addrlen要赋初值sizeof（addr）
   ```

5. **发起连接请求**

   ```c
   int connect(int sockfd,struct sockaddr *addr,socklen_t *addrlen)//阻塞函数
       //返回值：返回新的套接字（也是文件描述符），newsockfd（最终要读写的文件描述符），由服务端的ip和端口号组成。
       //参数说明：
       addr:传入参数，为服务器的ip和端口号
       addrlen:调用方必须初始化它为包含指向的结构的大小（以字节为单位）ADDR;返回时，它将包含对等地址的实际大小.即addrlen要赋初值sizeof（addr）
   ```

## 8.3.IP地址序转换函数

```c
typedef uint32_t in_addr_t
in_addr_t inet_addr(const char* cp);//把字符串形式的点十分ip地址转换为网络字节序的二进制
addr.s_addr.s_addr = inet_addr("127.0.0.1");


inet_aton(const char *cp,struct in_addr *inp);
////把字符串形式的点十分ip地址转换为ipv4地址结构体
```

## 8.3.套接字类型

- **流式套接字**：TCP

- **数据报套接字**：UDP

- **原始套接字**：IP,ICMP（ping命令），直接作用到网络层或者数据链路层
- **注意：**套接字属于一种特殊的文件描述符，因为是文件，所有也有打开关闭，读写操作，UNIX域套接字用于本地进程通信![image-20241004164910757](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20241004164910757.png)			

# ————服务器模型————

# 1.TCP通信流程![image-20240614122551493](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240614122551493.png)

# 2.TCP服务器的实现

```c
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define BACKLOG 5

/*main函数传参实现输入IP地址和端口号*/
int main(int argc, char *argv[]){
	int sockfd;
    char buf[128] = {0};
    int ret;
	struct sockaddr_in addr_s;

	if (argc < 3){
		fprintf(stderr,"%s<addr><port>\n",argv[0]);
		exit(0);
	}
    
    
	/*创建套接字*/
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd < 0){
		perror("socket");
		return 0;
	}
    
    /*绑定通信结构体*/
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(atoi(argv[2]));//把端口号转为网络字节序
	if (inet_aton(argv[1],&addr_s.sin_addr) == 0) {
		fprintf(stderr,"Invalid address\n");//不用perror是因为此函数出错不会设置errno
		exit(EXIT_FAILURE);
	}
	if (bind(sockfd,(struct sockaddr*)&addr_s,sizeof(addr_s)) < 0){
		perror("bind");
		return 0;
	}
    
    
    /*设置套接字为监听模式*/
	if (listen(sockfd,BACKLOG) < 0){
		perror("listen");
		return 0;
	}
    
    /*处理客户端的连接请求*/
	int newfd;
	newfd = accept(sockfd,NULL,NULL);
	if (newfd < 0){
		perror("accept");
		return 0;
	}
	printf("accept success\n");
	while(1){
		memset(buf,0,128);
		ret = read(newfd,buf,128);
		if (ret > 0){
			printf("buf= %s",buf);
		}else if(ret == 0){
			break;
		}else{
			perror("read");
			exit(0);
		}
	}
	close(newfd);
	close(sockfd);
	return 0;
}
```

# 3.TCP客户端的实现

```c
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
int main(int argc, char *argv[]){
	int sockfd;
	struct sockaddr_in addr_s;
	if (argc < 3){
		fprintf(stderr,"%s<addr><port>\n",argv[0]);
		exit(0);
	}
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1],&addr_s.sin_addr) == 0) {
		fprintf(stderr,"Invalid address\n");//不用perror是因为此函数出差不会设置errno
		exit(EXIT_FAILURE);
	}
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd < 0){
		perror("socket");
		return 0;
	}
	if (connect(sockfd,(struct sockaddr*)&addr_s,sizeof(addr_s)) < 0){
		perror("connect");
		return 0;
	}
	printf("connect success\n");
	char buf[128] = {0};
	while(1){
		printf("input->");
		fgets(buf,128,stdin);
		write(sockfd,buf,strlen(buf));
	}
	close(sockfd);
	return 0;
}

```



# 4.TCP并发实现

- **线程并发**
  线程设置自身为游离态预防僵尸线程。

- **进程并发**
  父进程不断等待接收客户端连接请求并创建子进程，子进程负责处理客户端请求。
  父进程利用SIGCHLD信号捕捉，捕捉函数里调用wait函数回收子进程来避免僵尸进程。
- **IO模型**

# 5.函数接口扩展

```c
1.用于TCP
ssize_t send(sockfd,buf,128,int flags)
ssize_t recv(sockfd,buf,128,int flags)
    //发送接收函数，前三个参数同read，write
    //成功返回实际发送/接收字节数，失败返回-1，并设置errno
    //flags：
    	1.为0，相当于write，read函数
    	2.recv的为MSG_PEEK:则每次读取数据后，数据还在输入队列，下次读的时候就是本次收到的新的紧跟在上一次数据的后面
        3.send的为MSG_OBB：处理带外（OOB）数据？？？

   
            
            
2.用于UDP
ssize_t sendto(sockfd,buf,128,int flags,struct sockaddr *dest_addr, socklen_t *addrlen)
ssize_t recvfrom(sockfd,buf,128,int flags,struct sockaddr *src_addr, socklen_t *addrlen)
            //前四个参数同send，recv一样
            //addr和addrlen分别为通信结构体和结构体宽度
均同read，write一样为阻塞函数！！
```

# 6.UDP通信流程

![image-20240617201507278](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240617201507278.png)

TCP协议（报文段）：

TCP的主要特点：

1.面向连接，每条连接只能是点对点（这里的一个点对应一个套接字），即一对一，调用socket函数的端点也叫socket

2.提高可靠交付的服务

3.全双工

4.面向字节流，以字节为单位，不管你是图片还是文本，还是声音，在tco来看就是一串无结构的字节流，但注意实际当中发送数据是一块一块的数据块！！

TCP是如何实现可靠传输的：

2.超时重传：发完一个数据包，等ACK确认包才开始下一次发送，计时器时间到没收到ACK，那么就超时重发。

3.确认丢失：接收方收到了，但ACK包发送过程中丢了，接收方不知道超时重发，所以接收方会接到重复的包，这时接收方会1.丢弃这个重复的分组，不向上层交付，但还会再向发送法发送ACK，以避免发送方又以为超时了

4.确认迟到：发送方会收到重复ACK，则直接丢弃。

提高传输效率：

![image-20240617220723731](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240617220723731.png)

在收到确认之前，发送方连续发出多个分组。

滑动窗口，ARQ协议：

![image-20240617221036914](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240617221036914.png)

# >>>>>>TCP/UDP协议介绍<<<<<<

# 1.TCP协议

## 1.1.TCP报文段首部格式

TCP首部：20字节固定大小，首部长度为4*n，其中n的值为首部长度这个字段的值

数据偏移：说明TCP数据从哪开始，从而推出TCP首部有多大

序号seq：随机生成，后面的依次递加

确认号ack：此次确认号为下一次数据包的序号

标志位：ACK大小的，SYN（置1表示请求连接，求情连接数据包不携带数据，但消耗一个序列号）,FIN（置1，表示断开了连接），PUSH表示把数据递交给应用层了

## 1.2.TCP连接管理

1.三次握手协议建立连接，

![image-20240617223804290](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240617223804290.png)

2.四次挥手断开连接

c->s:FIN = 1,seq = u,FIN报文段即使不携带数据，也消耗掉一个序列号

s->c：ACK = 1,seq = v, ack = u + 1,同时通知应用进程，TCP释放连接了

s->c:FIN = 1,ACK = 1,seq = w,ack = u + 1，可能有数据，也可能没数据，此时处于半连接状态

c->s:ACK = 1,seq = u + 1，ack = w + 1

最后：客户端再等待2MSL时间后彻底关闭

保活计时器：防止TCP长期的处于空闲状态

UDP的主要特点：

简单方便，但不可靠

首部只有8个字节

面向报文

一对一，一对多，多对一，多对多

对数据报文既不合并也不拆分，直接原样发送

IP协议：

IP协议的作用和意义

IP数据报首部：

Unix域套接字：

unix域流式套接字--TCP

```c
fd = socket(AF_UNIX,SOCK_STREAM,0)
```

unix域数据报套接字--UDP

```c
fd = socket(AF_UNIX,SOCK_DGRAM,0)
```

服务器流程：基本与TCP/UDP一致，不同之处在于绑定用的是套接字文件

通信结构体：

```c
for AF_UNIX, see unix(7) //man 7 unix

struct sockaddr_un {
   sa_family_t sun_family;               /* AF_UNIX */
   char        sun_path[108];            /* Pathname */
};
pathname:bind时创建，用完后要删除该文件
/*Binding  to a socket with a filename creates a socket in the filesystem that must be deleted by the caller when it is no longer needed (using unlink(2) or remove(3)).*/
```

# ————IO模型————

# 		基本概念

## 			同步和异步

1. 同步：进程执行到read，scanf时都是要等待有数据结果时才继续执行接下来的语句。
2. 异步：与同步的区别就是，不需要等待请求调用请求出最终结果，异步请求返回时一定不知道结果，还得通过其他机制来获取结果。

## 			阻塞与非阻塞

- 线程/进程的五个状态：新建态，就绪态，运行态，阻塞态，死亡状态

- 会进入阻塞态的情况：
  sleep函数调用；

  I/O阻塞：如read，scanf函数；

  线程试图得到一个锁；

  线程在等待某个触发条件；

- 可能阻塞的socket API：

  输入输出操作函数，accept接受连接，connect外出连接.


# 			2.五种I/O模型

 1）阻塞I/O：recvfrom，read，accept，connect的默认情况。

 2）非阻塞I/O（non-blocking）:内核没有数据报，返回**EAGAIN**,系统调用返回错误且为**EAFAIN**，则程序一直循环执行系统调用，当有数据时，内核在拷贝数据的那一小段时间函数仍然是阻塞的。

 3）多路复用I/O:I/O多路复用是一种在单个线程中管理多个输入/输出通道的技术，允许一个线程同时监听多个输入流（例如网络套接字、文件描述符等），并在有数据可读或可写时进行相应的处理，而不需要为每个通道创建一个独立的线程。即调用accept之前先看看能不能调用accept，不能就直接干其他的，而不是一来就直接调用accept，从而避免当没有客户端请求建立连接时被阻塞。

 4）信号驱动式I/O（半异步，但由于真正发生I/O操作的过程是同步过程，所以才算同步I/O）。

前四种均是同步I/O。

 5）异步I/O：函数一调用立马去干别的，内核等待数据拷贝数据都不阻塞进程。

# 		3.I/O多路复用select函数

```c
/* 函数的功能是监视多个文件描述符，直到一个或多个文件描述符准备就绪，或者超过了指定的超时时间。它允许程序同时等待多个输入/输出操作，而无需为每个操作使用单独的线程或进程。*/
/*原先的set，每个比特位上为0表示没该文件描述符，为1表示有该文件描述符，select把set集合重新设置一下，0表示没数据，1表示有数据。原先为0的比特位必为0，原先为1的比特位可能变成0*/
int select(int nfds,fd_set *readfds,fd_set* writefds,fd_set *exceptfds,struct tiemval *tiemout)
    //返回值:函数的返回值是一个整数，表示已就绪的文件描述符的数量。
    	1.-1:出错。
        2.大于0，则表示有文件描述符已就绪，并且等于 readfds、writefds 或 exceptfds 集合中已设置的文件描述符的数量之和。
        3.0，表示在指定的超时时间内没有文件描述符就绪。
    //参数说明:
    nfds：这是一个整数值，指定了被检查的文件描述符的范围.
    readfds：这是一个指向 fd_set 类型的指针，用于指定需要监视的可读文件描述符集合。如果某个文件描述符被设置为可读，并且其状态变为可读，则 select 返回时该文件描述符将被标记为可读。
	writefds：这是一个指向 fd_set 类型的指针，用于指定需要监视的可写文件描述符集合。如果某个文件描述符被设置为可写，并且其状态变为可写，则 select 返回时该文件描述符将被标记为可写。
	exceptfds：这是一个指向 fd_set 类型的指针，用于指定需要监视的异常文件描述符集合。这通常用于监视套接字上的带外数据或错误状态。
	timeout：这是一个指向 timeval 结构的指针，用于指定 select 函数等待的最长时间。
    		1.NULL,一直等待，直到有文件描述符就绪;
			2.0，立即返回，无论是否有文件描述符就绪。
            3.具体在值:先等待，时间到返回。
```

fd_set：比特位

| 0    | 0    | 0    | 1    | 1    | 0    | 1    | 0    |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |

优点：接口简洁，易于理解和上手,等待时间精确到微秒。

缺点：具体哪个文件描述符可读还需要自己遍历,而遍历是采用线性扫描的方式来遍历文件描述符集合。

# 		4.I/O多路复用poll函数

```c
1.int poll(struct pollfd *fds,nfds_t nfds,int timeout)
    //返回值:函数的返回值是一个整数，表示已就绪的文件描述符的数量。
    	1.-1:出错。
        2.大于0，则表示有文件描述符已就绪，并且等于 fds数组中revents的非0的元素的个数
        3.0，表示在指定的超时时间内没有文件描述符就绪。
    //timeout:单位毫秒
    //struct pollfd{
    	int fd;   //文件描述符
		short events;//请求的事件，传入参数
		short revents;//返回的事件，由系统返回，是传出参数
	 }
	//nfds:fds的个数，相当于struct pollfd数组的最后一个元素的下一个元素的下标。
	//timeout:-1阻塞，0不阻塞，超时时间
2.events事件类型:
	POLLIN:有数据可读
    POLLPRI:有紧急数据需要读取
    POLLOUT:文件可写
3.注意:
	POLLIN：这是一个宏，通常定义为 0x0001（或其他值，但通常是 2 的幂）。它表示对“可读”事件感兴趣。当执行 fds[n].revents & POLLIN 时，实际上是在检查 fds[n].revents 的最低位是否为 1。如果为 1，那么这意味着对应的文件描述符（fds[n].fd）在 poll 调用期间已准备好进行读取操作。
	举个例子，假设 fds[n].revents 的值为 0x0003（即二进制 0000 0011），这通常表示两个事件都发生了：POLLIN（数据可读，值为 0x0001）和 POLLOUT（数据可写，假设其值为 0x0002）。当执行 fds[n].revents & POLLIN（即 0000 0011 & 0000 0001）时，结果是 0x0001（或 1），这表示 POLLIN 事件已发生。
```

​	优点：poll没有使用fd_set，而是允许自己设定自定义长度监听集合数组，因此可以突破1024的监听限制。poll允许对集合中不同的文件描述符设置不同的监听事件，不像select一样多监听多个事件还要设置多个fd_set，因此比select更灵活。

​	缺点：与select类似，poll也需要将文件描述符数组从用户空间复制到内核空间，并且在返回后需要遍历整个数组来找到就绪的文件描述符。

# 		5.I/O多路复用epoll函数族

```c
/*创建epoll句柄*/
int epoll_create(int size);//size实际已被弃用

/*epoll句柄的控制接口*/
int epoll_ctl(int epfd,int op,int fd,struct epoll_event *event);
	//epfd:epoll句柄
	//op:动作，三个宏定义
			1.EPOLL_CTL_ADD:把新的fd加到epfd中
            2.EPOLL_CTL_MOD:修改监听事件
            3.EPOLL_CTL_DEL:从epfd中删除fd
    //event:告诉内核要监听什么事件，传递是地址
           typedef union epoll_data {
               void        *ptr;
               int          fd; /*文件描述符*/
               uint32_t     u32;
               uint64_t     u64;
           } epoll_data_t;

           struct epoll_event {
               uint32_t     events;      /* Epoll events */
               epoll_data_t data;        /* User data variable */
           };
2.epoll_event.events事件类型:
	EPOLLIN:有数据可读
    EPOLLPRI:有紧急数据需要读取
    EPOLLOUT:文件可写

/*返回可以进行I/O操作的文件描述符数量*/
int epoll_wait(int epfd,struct epoll_event *events,int maxevets,int timeout).....
  //参数说明:
          epfd：这是通过 epoll_create 或 epoll_create1 创建的 epoll 文件描述符。
		  events：这是一个指向 epoll_event 结构体的数组的指针，该数组用于存储 epoll_wait 返回的事件,即每个元素表示的是某个文件描述符的监听事件发生了就被加到数组中去，通过返回值遍历该数组，就可以知道哪个文件描述符的哪个事件发生了。此函数的events是传出参数。The buffer pointed to by events is used to return information from the ready list about file de‐ scriptors in the interest list that have some events available.
		  maxevents：这个参数告诉内核 events 数组的最大大小，即内核最多返回多少个事件。
		  timeout：这是等待的超时时间（以毫秒为单位）。如果设置为 -1，则 epoll_wait 会一直阻塞，直到有事件到来或者调用被中断。如果设置为 0，则 epoll_wait 会立即返回，不管是否有事件到来。  
  //返回值:epoll_wait 函数的返回值表示返回的事件数量,即events数组的实际大小。如果返回 -1，则表示发生错误，可以通过 errno 来查看具体的错误原因。返回0，超时
```

优点：当事件就绪时，内核直接将就绪的事件通知给用户空间，不需要像selcet和poll一样总去重复检查某个文件描述符的事件是否就绪。而且遍历是遍历返回的有事件就绪的events数组，避免了每次调用都需要遍历整个文件描述符数组的性能开销。

缺点：相对于select和poll，epoll的编程复杂度稍高。

# 		————套接字属性设置————

# 			1.选项级别

## 1.1.基本概念

- 设置套接字的选项对套接字进行控制
- 除了设置选项外，还可以获取选项
- 选项的概念相当于属性，所以套接字选项也可以是套接字属性
- 有些选项只可以获取，不可以设置
- 有些选项既可以获取也可以设置

## 1.2.常用选项级别

- **SOL_SOCKET**
  此级别的选项只作用于套接字本身
  
  ```shell
   The  socket  options listed below can be set by using setsockopt(2) andread with getsockopt(2) with the socket level set to SOL_SOCKET for all sockets.  Unless otherwise noted, optval is a pointer to an int.
  ```
- **SOL_LRLMP**
  此级别选项作用于IrDA协议
- **IPPROTO_IP**
  此级别选项作用于IPV4协议
- **IPPROTO_IPV6**
  此级别选项作用于IPV6协议
- **IPPROTO_TCP**
  此级别选项作用于流式套接字
  
  ```shell
  /*man tcp8/
  To  set or get a TCP socket option, call getsockopt(2) to read or setsockopt(2) to write the option with the option level argument set to IP‐PROTO_TCP.Unless otherwise noted, optval is a pointer to an int.  In addition, most IPPROTO_IP socket options are  valid  on  TCP  sockets.Most socket-level options utilize an int argument for optval.  For setsockopt(), the argument should be nonzero to enable a  boolean  option,or zero if the option is to be disabled.//1使能，0禁用
  ```
- **IPPROTO_UDP**
  此级别选项作用于数据报套接字


# 			2.SOL_SOCKET级别常用选项

![image-20240620105112498](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240620105112498.png)

# 			3.IPPROTO_IP级别常用选项

<img src="C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240620105216940.png" alt="image-20240620105216940" style="zoom:150%;" />

# 4.获取套接字属性

```c
int getsockopt(int sockfd,int level,int optname,void *optval,socklen_t *optlen);
	//参数说明:
		  sockfd:套接字文件描述符
            level:选项级别
            optname:要获取对应级别的选项名称
            optval:指向存放接收到的选项内容的缓冲区
		   optlen:传出参数，指向optval所指缓冲区的大小，即数据类型的宽度
    //返回值；
             成功返回0，失败返回-1，并设置errno
```

# 5.设置套接字属性

```c
/*套接字属性设置*/
int setsockopt(int sockfd,int level,int optname,const void *optval,socklen_t optlen);


/*举例：地址快速重用*/
int flag = 1
setsockopt(sockfd,SOL_SOCKET,SO_REUSERADDR,&flag,sizeof(int));
```

# 			4.广播与组播(只能UDP实现)

## 4.1.广播地址

一个网络内，主机号全为1的IP地址，发送端通过SETSOCKOPT设置好socket可以发送广播地址，才允许广播，不设置的话默认不能广播。

```c
int on = 1;
setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on));
```

## 4.2.组播地址

不同网络但同一组的主机都能收到。

## 4.3.组播的实现

1.创建用户数据报套接字。

2.加入多播组

```c
struct ip_mreqn{
    struct in_addr imr_multiaddr;/*IP组播地址*/
    struct in_addr imr_address;	/*本地接口的IP地址*/
    int            imr_ifindex;  /*本地网卡的编号*/
}
if (setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0){
    perror("setsockopt");
    exit(0);
}
```

3.绑定组播IP地址(不是本地ip)和端口

4.等待接收数据

# 						5.网络层原始套接字

```c
sockfd = socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
//protocol is the IP protocol in the IP header to be received or sent(接收或发送的数据包含IP数据报包头，TCP/UDP包头).  Valid values
      // for protocol include:

      //• 0 and IPPROTO_TCP for tcp(7) stream sockets;

      // • 0 and IPPROTO_UDP for udp(7) datagram sockets;

      //• IPPROTO_SCTP for sctp(7) stream sockets; and

      //• IPPROTO_UDPLITE for udplite(7) datagram sockets.

```

#### 			链路层层原始套接字:

```c

```

域名解析:

```c
struct hostent *gethostbyname(const char*)
struct hostent{
	char *h_name; //官方域名
    char **h_aliases; //别名，字符串数组，最后一个元素为null
    int h_addrtype;  //地址族，ipv4还是ipv6
    int h_length;//地址所占字节数
    char **h_addr_list;//地址列表，若干个地址
        
}
endhostent();

inet_ntoa(*(struct in_addr*)h_addr_list[i]);//转成十分点进制的ip地址
//释放gethostnyname创建的结构体内存
```

万维网服务器:

如何实现万维网服务器;



TCP实现传输文件:







