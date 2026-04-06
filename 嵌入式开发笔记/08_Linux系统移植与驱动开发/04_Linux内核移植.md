



5. 

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

## 6.2 Linux内核概述

### 6.2.1 Linux内核和操作系统

- 内核：内核是一个操作系统的核心，提供了系统的基本功能，是整个系统工作的基础，决定了整个系统的稳定性和性能
- 操作系统：操作系统是在内核的基础上增加了各种工具集，库，shell等

### 6.2.2 Linux层次结构![image-20240807171713149](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240807171713149.png)

### 6.2.3 Linux内核特点

- 代码结构清晰，模块化设计，因此开发效率高
- 支持丰富的硬件平台和网络协议
- 具有较高的稳定性
- 开放源代码
- 资料丰富，用户活跃
- 轻量化，以及模块编译带来的较强裁剪性

## 6.3 Linux内核文件制作

### 6.3.1 Linux内核源码结构

#### 6.3.1.1 平台相关代码

- **arch目录**：存放各个所支持的CPU架构的代码
- block目录：放的是支持磁盘操作的代码
- crypto目录：加密相关
- document，README：说明文档书，README宏观说明，document微观细节说明
- driver：设备驱动
- firmware：固件
- fs：文件系统
- include：头文件
- init：系统初始化
- ipc：进程间通信的代码，如共享内存，消息队列
- kernel：存放核心算法代码
- mm：内存管理相关代码
- net：网络协议实现的代码
- ... ....

### 6.3.2 Linux内核配置与编译

#### 6.3.2.1 Linux内核源码配置

1. 源码并不知道我们的处理器架构及交叉编译工具是什么，我们自己在顶层目录的Makefile中指定

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

#### 6.3.2.2 Linux内核源码编译

编译内核，第一次在ubuntu上编译Linux内核会提示缺少一个 `mkimage` 命令 该命令可在uboot源码中u-boot-2013.01/tools/目录下获取（**必须是编译后的 uboot**） 将该命令拷贝到 ubuntu 的`/usr/bin `目录下即可正确编译内核` $ sudo cp u-boot-2013.01/tools/mkimage /usr/bin/ `给该命令添加可执行权限 `$ sudo chmod 777 /usr/bin/mkimage `完成后回到内核的顶层目录下重新编译内核即可

```shell
make uImage -j4#编译选为“*”的选项到内核

make modules #编译内核模块，编译选为“M”的选项为独立模块
```

### 6.3.3 设备树

#### 6.3.3.1 设备树概念

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

#### 6.3.3.2 设备树语法

- **设备树文件**

  dts 设备树源文件

  dtsi 类似于头文件，包含一些公共的信息，可被其它设备树文件引用

  dtb 编译后的设备树文件

- **设备树语法**

  设备树的语法为树状结构，由一系列的节点和属性组成，根节点下包含子节点，子节点下还可以包含子节点，节点内部包含了对应设备的属性

#### 6.3.3.3 编译设备树

```shell
make dtbs      	
	编译设备树（将设备树源文件dts编译为二进制文件dtb）
```

### 6.3.4 Linux内核驱动移植

1.在make **menuconfig**界面中选中要安装的驱动，驱动可以是自己写的。

 2.在**设备树**中添加/修改相应的设备信息

 3.**重新编译**内核/设备树

# 7.rootfs移植

## 7.1.rootfs介绍

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



# >>>>>>移植总结>>>>>>