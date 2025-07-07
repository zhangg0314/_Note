# 下行抄表

## 1.`APP`介绍

`gatherexe`是用于执行下行抄表的`APP`。它主要是通过消息队列来接收其他进程发送的抄表消息来解析，并通过与本地载波模块串口或485串口来与电表进行交互。

1. `gatherexe`进程中创建了一个`dlt698_42`线程，他们之间通过`udpskt`（`UDP`套接字）来进行通信。主进程赋值监听消息队列或者检查抄表任务配置数据库来看是否有抄表任务需要执行，然后做进一步处理，通过`UDP`套接字发给线程，由线程来执行。
2. `dlt698_42`线程通过`select`函数监听来自上行的`gathexe`进程的消息和来自下行的串口消息。下行串口可以是485串口也可以是载波模块的串口。

## 2.`APP`执行流程

### 1.处理命令行参数

```c
static int PreProcess(int argc, char **argv)
```

1. 根据 `-t `选项传入的参数判断是执行哪个通道，`485I`还是`485II`或`plc`，从而生成不同的日志文件名。
2. 根据`LOGID_GATHEREXE`获取`OOP_LOG_CONFIG_PARAM`参数，从而获取当前`APP`对应日志ID的日志等级。
3. 如果传入的通道参数是485，则直接`return 0`；如果是`PLC`则进入步骤4。
4. 上电载波模块、载波模块复位引脚拉高，延时5秒，拉高复位引脚。
5. 根据波特率打开下行串口`fd_dnlnk`，至于是串口还是软路由，由环境变量`PLCTYPE`来决定，然后再打开上行串口（软路由）。
6. 在路由初始化前，需要加载并行抄表模式

## 3.`dlt698_42.cpp`文件

### 1.载波模块相关函数

这些函数主要供抄表调用，即`gatherexe`进程和`dlt698_42`线程中的函数调用。

```c++
/*
*
* 相关驱动位于bsp/SUNXI-BSP/blob/t3c-smios-test-develop/linux3.10/drivers/char/osal_devices/osal_plc_core.c
*
*/
int rtctrl_poweron(void); //此函数供抄表调用，用于上电载波模块
int rtctrl_reset(void);   // 此函数供驱动库调用，用于管脚复位载波模块（软路由为杀死进程方式）
uint32_t plc_baud(void); //获取载波模块波特率，有9600和115200，用这两种方式打开，总有一种会成功不乱码，从而确定模块波特率。
int get_moduleinfo(int trynum); //trynum重试次数，获取模块信息。
int open_fd(u_int32_t baud);//开启dlt698_42线程的下行抄表串口，上行与gatherexe通信的软串口（udpsocket）。
```

# 参数共享

主要利用`gdbm`数据库的技术，存储一些**各个进程**都需要用到的参数，如校时参数，表档案参数，通信参数等。

## 1.参数结构体定义

参考698.45协议定义，位于`param_struct_45.h`头文件中。
