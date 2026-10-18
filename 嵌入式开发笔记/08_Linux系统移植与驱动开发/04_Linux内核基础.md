# 认识内核

## 1. 程序分类

程序按其运行环境分为：

1. 裸机程序：直接运行在对应硬件上的程序。
2. 应用程序：只能运行在对应操作系统上的程序。

## 2. 计算机系统层次结构

计算机系统两种层次结构：

- 无操作系统的简单的两层结构（硬件+裸机程序），如单片机开发，
  芯片：`如STM32F103C8T6`、`HC32F460Axx`等，
  开发环境：`如IAR`、`Keil`等。

- 有操作系统的复杂的四层结构（应用层+中间层（功能库、HAL库）+系统层（操作系统内核）+硬件）

  芯片：`HC32F4A60`、全志、exynos4412等，

  开发环境：`linux`、`RT-Thread`、`RTOS`等。

## 3.操作系统概念

- 狭义的操作系统：给应用程序提供运行环境的**裸机程序**，也被称为操作系统内核。

- 广义的操作系统：一组软件集合，它包含：


1. 最核心的一个裸机程序  ----------内核 (kernel)
2. `app`开发常用的一些功能库（如：C语言标准函数库、线程库、C++标准类库、QT类库等等）
3. 一些管理用的**特殊app**（如桌面、命令行、app包管理器、资源管理器、系统设置、一些常用后台服务程序）

## 4. 内核实现模式

### 1.内核组成

操作系统最核心的那个裸机程序，主要负责硬件资源的驱动和管理。一个操作系统内核主要包括如下几个子模块：

1. **任务管理**
   多任务支持、任务调度、任务间通信。
2. **内存管理**
   物理内存管理，虚拟内存实现。
3. **设备驱动（可裁剪）**
   各种外部设备的I/O支持。
4. **网络协议支持(可裁剪)**
5. **文件系统支持（可裁剪）**
6. **启动管理**

### 2.内核实现模式

1. 单内核（宏内核）
   所有子模块代码编译到一个比较大的可执行文件（镜像文件）中，各子模块代码共用同一套运行资源，各模块间的交互直接通过函数调用来进行。
2. 微内核
   只将<u>任务管理、内存管理、启动管理最基本的三个子模块编译到一个微型的可执行文件中</u>，其它子模块则各自编译成独立的后台服务程序，这些服务程序与微型内核以及`app`间主要通过各种`IPC`进行通讯。

单内核特点：效率高，稳定性低，扩展性差，安全性高，典型操作系统：UNIX系列、Linux

微内核特点：效率低，稳定性高，扩展性高，安全性低，典型操作系统：Windows，QNX

## 5. 设备驱动程序

英文：Device Driver

简称：驱动（Driver）

一种添加到操作系统中的特殊程序，主要作用是协助操作系统完成应用程序与对应硬件设备之间数据传送的功能。简言之，设备驱动程序就是操作系统中“驱动”对应硬件设备使之能正常工作的代码。一个驱动程序主要完成如下工作：

1. **初始化设备**
   让设备做好开始工作的准备（open）
2. **读数据**
   将设备产生的数据传递给上层应用程序（read）
3. **写数据**
   将上层应用程序交付过来的数据传递给设备（write）
4. **获取设备信息**
   协助上层应用程序获取设备的属性、状态信息（ioctl）
5. **设置设备信息**
   让上层应用程序可以决定设备的一些工作属性、模式（ioctl）
6. **其它相关操作**
   如休眠、唤醒、关闭设备等

其中最核心的工作就是设备数据的输入和输出，因此计算机外部设备（外设）也被称为IO设备。



# Linux内核概述

## 1.内核和操作系统

- 内核
  内核是一个操作系统的核心，提供了系统的基本功能，是整个系统工作的基础，决定了整个系统的稳定性和性能。
- 操作系统
  操作系统是在内核的基础上增加了各种工具集，动态库静态库，shel运行环境l等。

## 2.`Linux`层次结构

![image-20240807171713149](..\figure\image-20240807171713149.png)

## 3.`Linux`内核特点

- 代码结构清晰，模块化设计，因此开发效率高
- 支持丰富的硬件平台和网络协议
- 具有较高的稳定性
- 开放源代码
- 资料丰富，用户活跃
- 轻量化，以及模块编译带来的较强裁剪性

## 4.内核源码结构

### 1.平台相关代码

- arch目录
  存放各个所支持的CPU架构的代码。
- block目录
  放的是支持磁盘操作的代码。
- crypto目录
  加密相关。
- document，README
  说明文档书，README宏观说明，document微观细节说明。
- driver
  设备驱动
- firmware
  固件
- fs
  文件系统
- include
  头文件
- init
  系统初始化
- ipc
  进程间通信的代码，如共享内存，消息队列的实现。
- kernel
  存放核心算法代码。
- mm
  内存管理相关代码
- net
  网络协议实现的代码
- ... ....

### 2.内核配置编译

#### 1.内核源码配置

1. 源码并不知道我们的处理器架构及交叉编译工具是什么，需要在顶层目录的Makefile中指定

   ```shell
   ARCH ?= arm
   CROSS_COMPILE ?= arm-none-linux-gnueabi-
   ```

2. 指定使用的处理器

   ```shell
   make exynos_defconfig
   ```

3. 修改配置

   ```txt
   “*”，内核中该功能被选中，相关代码会被编译进内核
   
   “ ”，内核中该功能不被选中，相关代码不会被编译进内核
   
   “M”，内核中该功能被选为模块（被编译为独立的模块）
   ```

4. 进入内核配置界面，配置内核，选择要安装的驱动。

   ```shell
   make menuconfig
   #使用make menuconfig配置的本质还是修改.config文件
   ```

#### 2.内核源码编译

编译内核，第一次在ubuntu上编译Linux内核会提示缺少一个 `mkimage` 命令 该命令可在uboot源码中u-boot-2013.01/tools/目录下获取（**必须是编译后的 uboot**） 将该命令拷贝到 ubuntu 的`/usr/bin`目录下即可正确编译内核` $ sudo cp u-boot-2013.01/tools/mkimage /usr/bin/ `给该命令添加可执行权限 `$ sudo chmod 777 /usr/bin/mkimage `完成后回到内核的顶层目录下重新编译内核即可。

```shell
make uImage -j4#编译选为“*”的选项到内核

make modules #编译内核模块，编译选为“M”的选项为独立模块
```

### 3.设备树

#### 1.设备树概念

设备树是一种描述硬件信息的数据结构，Linux内核运行时可以通过设备树将硬件信息直接传递给Linux内核，而不再需要在Linux内核中包含大量的冗余编码。内核中有设备驱动作为设备工作时的逻辑代码。具体信息在设备树里。比如：

```c
/*设备树代码*/
LED_ON = 0x0000001;
LED_init = 0x41000c40；//保存设备具体信息
 ... ... 
 ... ...
    
/*驱动代码*/
LED1 = LED_ON;//实现设备工作逻辑  
```

#### 2.设备树语法

- **设备树文件**

  dts 设备树源文件

  dtsi 类似于头文件，包含一些公共的信息，可被其它设备树文件引用

  dtb 编译后的设备树文件

- **设备树语法**

  设备树的语法为树状结构，由一系列的节点和属性组成，根节点下包含子节点，子节点下还可以包含子节点，节点内部包含了对应设备的属性。

#### 3.编译设备树

```shell
make dtbs      	
#编译设备树（将设备树源文件dts编译为二进制文件dtb）
```

### 4.内核驱动移植

1.在make **menuconfig**界面中选中要安装的驱动，驱动可以是自己写的。

 2.在**设备树**中添加/修改相应的设备信息。

 3.**重新编译**内核/设备树。

# 配置文件语法

## 1.`Kconfig`

- 图文并茂
  [把Linux驱动编译到内核的预备知识-Kconfig 文件语法_linux驱动kconfig-CSDN博客](https://blog.csdn.net/huilin9966/article/details/142586656)
- 只讲语法
  [【构建】Kconfig入门、常用语法学习笔记 | Jindu-Chen](https://jindu-chen.github.io/2024/01/05/【构建】Kconfig入门、常用语法学习笔记/)

## 2.`Makefile`

- `obj-y` 是一个在内核 Makefile 中常用的变量，用于指定一组默认的目标文件（.o 文件），这些文件将被编译并链接到最终的内核映像或模块中。`+=` 操作符用于追加列表，如果 `obj-y` 已经包含了一些目标文件，`mem.o` 和 `random.o` 将被添加到这个列表的末尾。
- `obj-$(CONFIG_TTY_PRINTK)` 是一个条件编译指令，它依赖于内核配置选项 `CONFIG_TTY_PRINTK` 的值。`CONFIG_TTY_PRINTK` 是内核配置中的一个选项，通常在内核的 `Kconfig` 文件中定义。

# 驱动编译加载

## 1.静态加载法

新功能源码与内核其它代码一起编译进`uImage`文件内，新功能源码与Linux内核源码在同一目录结构下：

1. 在`linux-3.14/driver/char/`目录下编写`myhello.c`，文件内容如下：

   ```c
   #include <linux/module.h>
   #include <linux/kernel.h>
   int __init myhello_init(void)
   {
   	printk("#####################################################\n");
   	printk("#####################################################\n");
   	printk("#####################################################\n");
   	printk("#####################################################\n");
       printk("myhello is running\n");
   	printk("#####################################################\n");
   	printk("#####################################################\n");
   	printk("#####################################################\n");
   	printk("#####################################################\n");
   	return 0;
   }
   void __exit myhello_exit(void)
   {
   	printk("myhello will exit\n");
   }
   MODULE_LICENSE("GPL");
   module_init(myhello_init);
   module_exit(myhello_exit);
   ```

2. 给新功能代码配置Kconfig（让make menuconfig中可以选新模块）

   ```bash
   #进入myhello.c的同级目录
   cd  ~/fs4412/linux-3.14/drivers/char
   
   #打开Kconfig文件
   vim Kconfig
   
   #添加如下内容：
   config MY_HELLO
   	tristate "This is a hello test"
   	help
   		This is a test for kernel new function
   ```

3. 给新功能代码改写Makefile（将选中新模块与新模块代码关联）

   ```shell
   #进入myhello.c的同级目录
   cd  ~/fs4412/linux-3.14/drivers/char
   
   vim Makefile
   #拷贝18行，粘贴在下一行，修改成：
   obj-$(CONFIG_MY_HELLO)     += myhello.o
   ```

4. make menuconfig  界面里将新功能对应的那项选择成<*>

   ```shell
   cd  ~/fs4412/linux-3.14
   make menuconfig
   #make menuconfig如果出错，一般是两个原因：
   #1. libncurses5-dev没安装
   #2. 命令行界面太小（太矮或太窄或字体太大了）
   ```

5. `make uImage`

6. `cp arch/arm/boot/uImage /tftpboot`

7. 启动开发板观察串口终端中的打印信息

## 2.动态加载法

新功能源码与内核其它源码不一起编译，而是独立编译成内核的插件(被称为内核模块）文件`.ko`

### 1.同一目录

新功能源码与Linux内核源码**在同一目录**结构下时：

1. 给新功能代码配置`Kconfig`

2. 给新功能代码改写Makefile

3. `make menuconfig` 界面里将新功能对应的那项选择成<M\>，确保新功能源码文件不会同内核一起编译进`uImage`，而是留到后续执行`make moudules`时进行编译为`.ko`文件。

4. `make uImage`

5. `cp arch/arm/boot/uImage /tftpboot`

6. `make modules(顶层目录下执行)`

   `make modules`会在新功能源码.C的同级目录下生成相应的同名`.ko`文件（生成的`ko`文件只适用于开发板`linux`）。注意此命令执行前，开发板的内核源码已被编译即`uImage`已经被编译好存在了

### 2.不同目录

新功能源码与Linux内核源码**不在同一目录**结构下时

1. `cd  ~/fs4412`

2. `mkdir mydrivercode`

3. `cd mydrivercode`

4. `cp  ../linux-3.14/drivers/char/myhello.c`

5. vim **Makefile**（**自己编写或者公司提供的辅助makefile,位于其他目录**）

   ​	虽然是不同目录，但make原理也同位于Linux顶层目录下的Makefile去make其他目录下的模块文件一样，只不过要额外自己编写一个makefile来协助顶层目录的makefile。比如告诉Linux内核的顶层Makefile模块源码目录的路径。

   ​	而第二次执行这个自己编写的协助的makefile时相当于执行模块源码目录下的Makefile本来就要有的内容`obj-m += hello.o`,即与模块源文件在同一目录下的Makefile也要有这一行生成.o的命令。

   ​        此Makefile的编写可以参照Linux源码目录下与各种模块处于同一目录下的Makefile，因为无论模块文件是否与Linux内核源码处于同一目录，其模块源码目录下的`Makefile`都是为了协助Linux顶层目录下的Makefile完成编译生成`.ko`文件的。

   ```shell
   ifeq ($(KERNELRELEASE),)  #make此Makefile时，第一次进入此makefile，满足此条件。
   	ifeq ($(ARCH),arm) #生成arm平台.ko文件
   		KERNELDIR ?= 目标板linux内核源码顶层目录的绝对路径
   		ROOTFS    ?= 目标板根文件系统顶层目录的绝对路径
   	else               #生成x86平台.ko文件
   		#此目录放的是x86平台内核源码中的一些Makefile以及一些脚本文件等。
   		KERNELDIR ?= /lib/modules/$(shell uname -r)/build
   	endif
   	PWD := $(shell pwd)  #获取当前路径，即当前新功能源码及当前makefile所在路径
   modules:
   	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules #make默认生成的目标
   	#把当前路径作为参数传给Linux源码目录顶层目录下的Makefile并执行make，当make执行Linux顶层目录下的Makefile时，同静态加载模块时一样，会执行模块源码目录下的Makefile，此时又会回到当前目录下，第二次执行当前目录下的Makefile，此时执行的是else的内容，即生成模块源码对应的.o文件，然后再生成.ko文件，然后再传到当前目录下。
   modules_install:
   	$(MAKE) -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_PATH=$(ROOTFS) modules_install
   clean:
   	rm -rf  *.o  *.ko  .*.cmd  *.mod.*  modules.order  Module.symvers   .tmp_versions
   
   else
   	obj-m += hello.o#同静态加载以及同目录动态加载时的makefile改写一样的内容，这个是必须的！！
   endif
   ```

6. `make modules`（生成的`ko`文件适用于主机`ubuntu linux`）

7. `make ARCH=arm modules`（生成的`ko`文件适用于开发板`linux`，**注意此命令执行前，开发板的内核源码已被编译**）

### 3.运行`ko`文件

- 主机`ubuntu`下使用`ko`文件

  ```shell
  sudo insmod ./???.ko  #此处为内核模块文件名，将内核模块插入正在执行的内核中运行 ----- 相当于安装插件
  
  lsmod | grep ???      #查看已被插入的内核模块有哪些，显示的是插入内核后的模块名
  
  sudo rmmod ???        #，此处为插入内核后的模块名，此时将已被插入的内核模块从内核中移除 ----- 相当于卸载插件
  
  sudo dmesg -C         #清除内核已打印的信息
  
  dmesg                 #查看内核的打印信息
  ```

- 开发板Linux下使用ko文件

  ```shell
  ##先将生成的ko文件拷贝到/opt/4412/rootfs目录下：
  cp ????/???.ko  /opt/4412/rootfs
  
  #在串口终端界面开发板Linux命令行下执行
  insmod ./???.ko   #将内核模块插入正在执行的内核中运行 ----- 相当于安装插件
  lsmod             #查看已被插入的内核模块有哪些
  rmmod ???         #将已被插入的内核模块从内核中移除 ----- 相当于卸载插件
  
  #内核随时打印信息，我们可以在串口终端界面随时看到打印信息，不需要dmesg命令查看打印信息
  ```

------

# rootfs移植

## 1.rootfs介绍

根文件系统是内核启动后挂载的**第一个**文件系统，其他文件系统都是在它基础上加载构建的，它是其他文件系统的根,系统引导程序会在根文件系统挂载后从中把一些基本的初始化脚本比如(比如自启动脚本))和服务等加载到内存中去运行，根文件系统本质就是一些文件。

## 7.2.rootfs文件目录分析

### 7.2.1.直接mkdir建立的空目录

- **dev** 
  存放设备文件信息(内核启动后会将设备信息写入该目录，因为内核加载时会加载驱动模块，驱动模块里面有自动mknod的代码)
- **proc** 
  存放进程相关文件(内核启动后会将启动的一些进程的进程信息写入该目录)
- **【mnt】** 
  挂载目录(非必要)
  windows某个文件属于某个磁盘，但Linux是某个磁盘属于某个文件，比如插入一个u盘给linux
- **【root**】 
  超级用户家目录(非必要)
- **sys** 
  驱动相关文件(内核启动后会将驱动模块，总线等信息写入该目录)
- **tmp**
  存放临时文件
- **var**

### 7.2.2.需要进行编译配置的目录制作

1. **bin**
   (存放mv,ls，cp以及**busbox的可执行文件**等，存放的Linux的shell命令是busybox（也是elf格式的文件）的**软链接**

2. **sbin**
   (super bin,存放比较重要的shell命令)、

3. **usr**
   （也包含bin和sbin）

4. **linuxrc**

   busybox的软连接，是linux内核运行起来后运行的**第一个应用程序**。
   所有设置uboot启动参数时有句命令是`bootcmd  init = /linuxrc`

   ```shell
   1.
   	先下载BuysBox,解压编译
   
   2.
   	make menuconfig #一般使用默认配置，需要修改的是编译器类型
   	make            #编译并生成BuysBox可执行文件
   	
   	make install   #生成BuysBox到各种命令的软链接，生成bin、sbin、usr、linuxrc各目录并放好各种软连接到当前目录下的_install目录下
   ```

5. **lib**
   arm架构的动态库文件，**可以直接复制交叉编译工具链里面的,**开发板上只需要有动态库就可以，因为开发板上只能运行程序，不能编译程序，而静态库是编译时链接(库生成的二进制代码被包含在elf文件中)，动态库是运行时链接。

6. **etc**
   内核配置文件，如init.d启动目录，里面可以存放一些开机就启动的shell命令来作为自启动，是一些文本文件（不区分架构），可以直接复制ubantu下的

# 6.Linux内核及rootfs移植

## 6.1 Linux内核及rootfs安装

### 6.1.1 tftp加载Linux内核及rootfs

1. 将资料中“Linux 内核镜像”目录的“uImage”和“exynos4412-fs4412.dtb”拷贝到 ubuntu 中 tftp 工作目录下 将资料中“根文件系统镜像”目录下的“ramdisk”也拷贝到 ubuntu 中 tftp 工作目录下。并修改这些文件的权限，重启 tftp 服务器。

2. 连接开发板与电脑，在 uboot 交互模式下，设置 uboot 的启动参数。

   ```shell
   1.
   	setenv ipaddr 192.168.0.102  #开发板的ip，需要与电脑处于同一网段下
   2.
   	setenv serverip 192.168.0.200 #tftp服务器，即电脑ip
   
   
   3.
   	setenv bootcmd 
   tftp 0x41000000 uImage\;  #tftp下载linux内核
   tftp 0x42000000 exynos4412-fs4412.dtb\;#tftp下载dtb
   tftp 0x43000000 ramdisk.img\;  #tftp下载rootfs
   bootm 0x41000000 0x43000000 0x42000000 
   4. 
   	setenv bootargs 
   root=/dev/nfs 					nfsroot=192.168.0.200:/opt/4412/rootfs/ rw
   console=ttySAC2,115200 #使用串口2，波特率115200
   init=/linuxrc 
   ip=192.168.0.102 #开发板地址
   ```

### 6.1.2 EMMC加载Linux内核及rootfs

1. 把linux，dbt，rootfs利用tftp下载到内存

2. 通过**mmc write 0**把内存的linux，dbt，rootfs写入到mmc的指定扇区

   ```shell
   mmc write 0 0x41000000 0x800 0x2000
   
   mmc write 0 0x41000000 0x2800 0x800
   
   mmc write 0 0x41000000 0x3000 0x2000
   ```

3. uboot自启动，修改uboot启动参数

   ```shell
   setenv bootcmd 
   'mmc read 0 0x41000000 0x800 0x2000\;  #EMMC加载linux内核
   mmc read 0 0x42000000 0x2800 0x800\;#EMMC下载dtb
   mmc read 0 0x43000000 0x3000 0x2000\;  #EMMCj加载rootfs
   bootm 0x41000000 0x43000000 0x42000000'  
   ```

### 6.1.3 tftp加载内核nfs挂载rootfs

1. uImage和dtb放到tftp服务器，开发板通过tftp加载Linux内核

   ```shell
   setenv bootcmd 
   tftp 0x41000000 uImage\;  #tftp下载linux内核
   tftp 0x42000000 exynos4412-fs4412.dtb\;#tftp下载dtb
   bootm 0x41000000 - 0x42000000 #rootfs没在内存，没用位置，故要写-
   ```

2. rootfs放到nfs服务器，开发板通过nfs自动挂载rootfs

**作用：**

​	rootfs，开发板与ubantu实时共享，在ubantu上写应用程序直接编译，开发板上也能看到编译文件，不用再用loadb传到开发板了，这样方便开发调试程序。

开发板没有操作系统是，要通过loadb传.bin文件来运行程序，但有操作系统且挂载后可以直接运行linux上的可执行文件，./test来运行。

# 内核时钟

## 11. 时钟中断

硬件有一个时钟装置，该装置每隔一定时间发出一个时钟中断（称为一次时钟嘀嗒-tick），对应的中断处理程序就将全局变量jiffies_64加

`jiffies_64`是一个全局64位整型, jiffies全局变量为其低32位的全局变量，程序中一般用jiffies

HZ：可配置的宏，表示1秒钟产生的时钟中断次数，一般设为100或200

## 12. 延时机制

1. 短延迟：忙等待

   ```c
   1. void ndelay(unsigned long nsecs)
   2. void udelay(unsigned long usecs)
   3. void mdelay(unsigned long msecs)
   ```

2. 长延迟：忙等待

   使用jiffies比较宏来实现

   ```c
   time_after(a,b)    //a > b
   time_before(a,b)   //a < b
   
   //延迟100个jiffies
   unsigned long delay = jiffies + 100;
   while(time_before(jiffies,delay))
   {
       ;
   }
   
   //延迟2s
   unsigned long delay = jiffies + 2*HZ;
   while(time_before(jiffies,delay))
   {
       ;
   }
   ```

3. 睡眠延迟----阻塞类

   ```c
   void msleep(unsigned int msecs);
   
   unsigned long msleep_interruptible(unsigned int msecs);
   ```


延时机制的选择原则：

1. 异常上下文中只能采用忙等待类
2. 任务上下文短延迟采用忙等待类，长延迟采用阻塞类

## 13. 定时器

（1）定义定时器结构体

```c
struct timer_list 
{
	struct list_head entry;
	unsigned long expires;  // 期望的时间值 jiffies + x * HZ
	void (*function)(unsigned long); // 时间到达后，执行的回调函数，软中断异常上下文
	unsigned long data;
};
```

（2）初始化定时器 

```c
init_timer(struct timer_list *timer);
```

（3）增加定时器 ------ 定时器开始计时

```c
void add_timer(struct timer_list *timer);
```

（4）删除定时器 -------定时器停止工作

```c
int del_timer(struct timer_list * timer);
```

（5）修改定时器 

```c
 int mod_timer(struct timer_list *timer, unsigned long expires);
```



```c
定义struct timer_list tl类型的变量


init_timer(...);//模块入口函数

//模块入口函数或open或希望定时器开始工作的地方
tl.expires = jiffies + n * HZ //n秒
tl.function = xxx_func;
tl.data = ...;

add_timer(....);


//不想让定时器继续工作时
del_timer(....);

void xxx_func(unsigned long arg)
{
	......
	mod_timer(....);//如需要定时器继续隔指定时间再次调用本函数
}
```





















