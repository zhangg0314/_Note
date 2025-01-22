# 嵌入式系统移植导学

## 系统移植本质

安装系统：在基于ARM处理器的开发板上安装Linux操作系统。

## 系统移植目的

1. 不同架构的处理器指令集不兼容，即便是相同的处理器架构，板卡不同驱动代码也不兼容。

2. 当Linux系统装上后，去移植一些应用层的东西，凡是二进制的文件包括ELF格式的，都区分X86和ARM架构，因此涉及到二进制文件的都需要进行移植。

3. Linux是一个通用的内核并不是为某一个特定的处理器架构或板卡设计的，所以从官方获取Linux源码后我们要先经过相应的配置使其与我们当前的硬件平台相匹配后才能进行编译和安装。


## 系统移植过程

1. 准备Linux内核镜像、SD卡启动盘。
2. 通过拨码开关选择启动方式（SD启动）。
3. 通过SD卡中的引导程序安装系统。
4. 安装Linux驱动程序。
5. 安装Linux应用程序。

## ★★★开发板启动过程★★★

1. 开发板上电后，首先第一个运行的程序是处于地址`0x00000000`的SOC内部`iROM`（内部只读存储器）中固化的程序(`BL0`)，它初始化基本的系统功能，如时钟和堆栈。

2. `iROM`从一个特定的启动设备加载`BL1`映像到内部256kb `SRAM`。通过OM （Operating Mode）引脚选择启动设备。根据安全启动键值，`iROM`对`BL1`映像进行完整性检查。
3. `BL1`初始化系统时钟和DRAM控制器。初始化DRAM控制器后，将OS映像从引导设备加载到`DRAM`中。根据安全启动密钥值，`BL1`可以对操作系统映像做一个完整性检查
4. 引导完成后，`BL1`跳转到操作系统。![image-20240804155941472](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240804155941472.png)

![image-20240804155143148](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240804155143148.png)

U-Boot开始运行后：
首先对开发板上的软硬件环境做进一步初始化，然后将linux内核、设备树(dtb)、根文件系统(rootfs)从外部存储器（或网络）搬移到内存，然后跳转到linux运行。

linux开始运行后先对系统环境做初始化，当系统启动完成后，Linux再从内存中（或网络）挂载根文件系统

# 服务器配置

## TFTP服务器

### 安装服务器

```shell
sudo apt-get install tftpd-hpa
```

### 配置服务器文件

```shell
# /etc/default/tftpd-hpa //配置文件的位置

TFTP_USERNAME="tftp"  #用户名
TFTP_DIRECTORY="/tftpboot" #工作路径，即要传输的文件所在目录
TFTP_ADDRESS="0.0.0.0:69"  #ip地址和端口号
TFTP_OPTIONS="-c -l -s"    #选项,类似于命令的选项，修改服务器的权限

#重启服务器，使配置生效
sudo service tftpd-hpa restart

#每次开启ubantu都要重启！！
```

## NFS服务器

nfs（Network File System）即网络文件系统，其基于UDP/IP，使用nfs能够在不同计算机之间通过网络进行文件共享，能使使用者访问网络上其它计算机中的文件就像在访问自己的计算机一样。即不需要总频繁的编译再下载到开发板上运行，而可以直接在电脑上现编译好，开发板现执行。

### 安装服务器

```shell
sudo apt-get install nfs-kernel-server
```

### 修改配置文件

```shell
# Example for NFSv2 and NFSv3:
# /srv/homes       hostname1(rw,sync,no_subtree_check) hostname2(ro,sync,no_subtree_check)
#
/opt/4412/rootfs   *(rw,sync,no_subtree_check,no_root_squash)

#重启服务器，使配置生效
sudo service nfs-kernel-server restart

#每次开启ubantu都要重启！！
```

# 交叉编译工具链

## 交叉编译工具链内容

### 交叉编译工具

这些工具位于bin目录下。

- arm-none-linux-gnueabi-**gcc**

- arm-... ...-**readelf**

- arm-... ...-**gdb**

- **size**：列出ELF中能够直接在cpu上运行的段的每一段的大小，不包括elf头，调试信息，符号表等段。

- **nm**：列出目标文件的符号表里的符号，如函数名，汇编标识符start,end等。

- **strip**：在生成elf文件时，系统会生成符号表，但我们用该命令删除elf文件的符号段且不会影响程序的执行，以此减少所占用空间，**这对嵌入式程序精简很重要**。通过file命令结果最后一句话来看文件是否被瘦过身。

- **objdump**：显示elf文件信息 **-d：反汇编**，机器码->汇编语言 ，

- **objcopy**：转换文件格式，用于把elf文件转为bin格式

  ```shell
  objcopy --gap-fill=0xff -O binary a.out a.bin
  #--gap-fill:用0xff去覆盖掉elf中除了数据段和代码段的其他段
  #-O :目标格式为binary
  ```

### 库

已经被编译成二进制文件但还未进行链接的动态库，且是ARM架构的库，只能在ARM上运行。

## ELF文件格式

ELF格式是Linux平台上应用最广泛的二进制工业标准之一

ELF格式的文件内包含了很多个段不同的段存储了不同的信息；因为ELF格式的文件要通过Linux系统的加载和管理才能运行，所以除了最基本的代码段和数据段之外，其中还存储了很多其它的信息，如符号表、调试信息等。**运行在linux之上**![image-20240803172533637](C:\Users\z3254406361\AppData\Roaming\Typora\typora-user-images\image-20240803172533637.png)

## 4.3 ELF相关命令

- file + 文件名
- readelf+文件名；查看elf文件详细信息

## 4.4 BIN文件格式

BIN文件一般是直接运行在CPU之上的可执行文件，文件内只包含了CPU能够直接识别和运行的指令和数据，不包含其它系统相关的信息。**运行在没有操作系统的裸机之上**

# U-Boot移植

## 什么是Bootloader

是在操作系统运行之前运行的一小段代码，用于将软硬件环境初始化到一个合适的状态，比如初始化时钟，中断，`DRAM`等外设为操作系统的加载和运行做准备（其本身不是操作系统），在把Linux从Flash拷贝到`DRAM`中，最后再启动Linux内核。

Bootloader是启动引导程序的统称，嵌入式Linux常用的Bootloader是U-Boot，类似于操作系统与Linux的概念。

## Bootloader基本功能

- 初始化软硬件环境
- 引导加载Linux内核
- 给Linux内核传参
- 执行用户命令

## U-Boot启动过程做了哪些事

第一阶段：初始化时钟，关闭看门狗，关中断，关闭MMU,TLB,DCACHE,ICACHE等初始化SDRAM，初始化NAND FLASH等。

第二阶段：初始化一个串口，检测系统内存映射，将内核映像和根文件系统映像从Flash上读到DRAM空间中，为内核**设置启动参数**，调用内核。

## SD卡刷U-Boot

### SD卡存储结构

SD卡的存储以**扇区为单位**，每个扇区的大小为**512Byte**,，其中零扇区存储分区表（即分区信息）,后续的扇区可自行分区和格式化；		若选择SD卡启动，处理器上电后**从第一个扇区开始**将其中的内容搬移到内存，所以我们把U-Boot放到从第一个扇区开始之后的空间， 之后的空间根据个人需求可进行分区和格式化。

### 刷卡步骤详解

1. 执行如下命令，制作一个 1M 的空镜像（用于擦除 SD 卡中原有的数据）

   ```shell
   sudo dd if=/dev/zero of=clear.bin count=2048
   #dd就是用于制作文件，if == inputfile， /dev/zero根目录下系统自带的文件，of = outfile， clear.bin制作的可自定义的文件名
   #2048 * 512 = 1M
   ```

2. 由于刷卡时，刷写程序默认是从第0块区开始刷，但uboot需要放到第1块区，而一块区大小为512字节，故需要先往里面刷一块512字节的程序来填充第0块。故在终端输入如下命令，制作一个 512 字节的空镜像

   ```shell
   sudo  dd  if=/dev/zero  of=zero.bin  count=1
   ```

3. 执行如下命令，将uboot追加到zero.bin之后合并生成win-u-boot-fs4412.bin

   ```shell
   cat  zero.bin  u-boot-fs4412.bin  > win-u-boot-fs4412.bin
   ```

4. 把生成的win-u-boot-fs4412.bin和clear.bin拷贝到windows下

5. 将SD卡插入到电脑上（卡槽/USB读卡器均可），使其在windows下识别（若不识别，可能是已经在ubuntu中识别了，在可移动设备中将其断开即可）

6. 利用`Win32DiskImager`把**clear.bin**和**win-u-boot-fs4412.bin**依次刷入SD卡。

## EMMC加载uboot

1. 先利用TFTP把uboot加载到内存。
2. emmc open 0 ，打开EMMC引导分区
3. mmc write 0 0x41000000 0x0 0x800，把uboot刷到EMMC的0扇区。**注意uboot刷到SD卡，刷到第1块，而刷到EMMC，刷到第0块**
4. emmc close 0，关闭EMMC引导分区

## U-Boot的使用

`uboot`环境变量`bootargs`是uboot传递给Linux的自启动参数，用于告诉Linux内核接下来该怎么初始化，怎么运行等。

```shell
setenv bootargs root=/dev/nfs 	nfsroot=192.168.0.200:/opt/4412/rootfs/ rw console=ttySAC2,115200 init=/linuxrc ip=192.168.0.102 
#root:告诉linux内核通过什么方式挂载根文件系统
#nfsroot:根文件系统所处主机以及所处路径
#rw:开发板对文件系统里文件的读写权限
#console:告诉linux打印信息通过哪个串口打印，并设置打印波特率
#init:告诉linux内核init启动的第一个进程在哪里,负责初始化系统设置、加载驱动程序、设置文件系统和启动系统服务等工作。
#ip:告诉linux内核自己的ip地址是多少，设置好后可以通过ifconfig查看。
```

## U-Boot文件制作

### uboot源码特点

- 代码结构清晰
- 支持丰富的处理器与开发板，**易于移植**
- 支持丰富的用户**命令**
- 支持丰富的**网络协议**
- 支持丰富的文件系统，文件系统就是一种管理和访问磁盘的软件
- 支持丰富的设备驱动
- 更新活跃，用户较多，资料丰富
- 开放源码，较高的稳定性
- 不具有通用性（不同的处理器，开发板uboot不能通用）

### uboot源码结构

#### 平台相关代码

​	即与CPU架构或开发板硬件相关的源码，硬件的改动对应的代码也要改动。

- **api目录**：上层接口
- **arch目录**：存放各种架构的处理器如ARM，X86等目录，如果只要用ARM，则把其他文件删了就留下ARM也许。ARM里面有个CPU目录，里面有ARM架构体系的各种型号CPU。选择与自己**开发板CPU架构和型号匹配**的文件进行编译即可。
- **board**：存放不同开发板类型的代码文件。由于哪怕CPU架构和型号一样但开发板不一样，同一代码也不能通用，故还要选择**开发板类型匹配**的文件来进行编译

#### 平台无关代码

- common：存放uboot命令实现的.c文件
- COPYING：版权信息文件
- CREDITS：uboot贡献人员名单
- disk：对磁盘的支持文件
- doc：README：说明书，说明文档
- drivers：驱动
- dts：设备树
- fs：file system文件系统

### uboot的配置与编译

理想状态下，即处理器架构型号，以及开发板类型，uboot源码中都有，那么不需要我们去修改代码来匹配我们自己的开发板，可以直接进行配置编译。

#### uboot配置

1. **指定当前使用的硬件平台和CPU架构**

   ```shell
   make <board_name>_config 
   
   #注1：<board_name>为当前使用的开发板的名字
   
   #注2：执行该命令的前提是uboot源码支持该开发板
   
   #注3：该命令必须在uboot源码的顶层目录下执行
   
   ifeq (arm,arm)
   	CROSS_COMPILE ?= arm-none-linux-gnueabi-
   endif
   
   #example
   make origen_config
   ```

2. **指定编译uboot源码使用的编译器**

   ```shell
   在uboot源码顶层目录下的Makefile中指定(CROSS_COMPILE变量)
   
   
   #example
   CROSS_COMPILE ?= arm-none-linux-gnueabi-
   ```

#### uboot编译

1. **编译uboot**

   ```shell
   make -j4 
   #-j jobs,指定并行编译的进程个数
   
   #注1：该命令必须在uboot源码的顶层目录下执行
   
   #注2：该命令执行后在uboot源码顶层目录下生成u-boot.bin 
   ```

2. **清除编译过程中生成的中间文件**

   ```shell
   make clean
   
   make distclean
   
   #注1：该命令必须在uboot源码的顶层目录下执行
   ```

### uboot移植——借鉴

要先修改一系列文件来适配fs4412，才能进行配置编译。

1. **添加Board信息**
   	因为uboot源码并不支持我们的开发板，这里我们需要从源码支持的开发板中找一个硬件与我们最类似的，在其基础上进行修改，这里我们参考的是samsung公司的origen。

   ```txt
   直接将origen目录复制一份改目录名为fs4412，修改目录中的.c和.h的文件名，因为修改了文件名，所以对应的
   
   Makefile内容也要修改，然后再修改.h头文件中的内容中打印开发板名字的宏定义。最后再在boards.cfg中添加fs4412的相关信息。
   ```

2. **添加三星加密引导方式**（因芯片而异，不需要掌握）

   ​    考虑芯片启动的安全性，Exynos4412需要三星提供的初始引导加密后我们的u-boot才能被引导运行，所以我们需要在uboot源码中添加三星提供的加密处理代码。添加完后，又因为添加的加密文件也要编译，所以对应的Makefile也要修改。

3. **添加调试代码（点灯法）**

   ​    很多时候我们不确定uboot是否已经在板子上运行，所以我们在uboot源码中添加一段代码使板子上的LED点亮，这样如果看到LED亮的话就表示uboot已经在运行了

   ​    打开uboot启动后的第一段代码

   ```shell
   vi arch/arm/cpu/armv7/start.S
   ```

   ​    在第134行后添加如下代码（即点亮LED2），然后保存退出

   ```asm
   ldr r0, =0x11000c40
   
   ldr r1, [r0]
   
   bic r1, r1, #0xf0000000
   
   orr r1, r1, #0x10000000
   
   str r1, [r0]
   ```

4. **添加编译脚本**

   ​    使用make命令编译时只链接uboot源码中的相关代码,而我们添加的初始引导加密的代码不会被连接到u-boot.bin中，所以这里我们自己编写编译脚本build.sh，这个脚本    中除了对uboot源码进行配置和编译外还将初始引导加密代码链接到了u-boot.bin上,最终生成一个完成的uboot镜像u-boot-fs4412.bin。

5. **实现串口输出**

   ​	虽然uboot已经能在开发板上加载运行，但是此时的uboot还不能在终端上打印信息，原因在于uboot源码中对UART的配置与我们实际的硬件不匹配。

### uboot移植——借鉴步骤总结

1. **明确需求**：把Linux内核正常运行起来，先要移植uboot

2. **准备工作**：先去官网下载U-Boot源码压缩包，然后解压

3. **确认芯片是否支持**：确认芯片架构和cpu类型（arch/arm/cpu/armv7）

   ```txt
   支持：goto 4
   不支持：找最新的uboot源码/找芯片厂家要
   ```

4. **确认板子是否支持**：

   ```txt
   支持：直接配置编译
   不支持：找最新的uboot源码/找板子厂家要/自己移植 goto 5
   ```

5. **代码移植：**

   ```
   借鉴-->创新
   1.借鉴：从源码支持的开发板中找一个硬件与我们最类似的，在其基础上进行修改，比如我们参考的是samsung公司的origen
   	1)把board目录下的origen目录直接复制一份并重命名为fs4412,然后重命名该目录里的.c文件，由于修改了文件名，故此时同目录下的makefile文件内容要修改。
   	2)进入include/configs目录，复制一份origen.h头文件并重命名为fs4412.h，并修改.h头文件里面内容，把origen改为fs4412
   	3)在顶层目录的board.cfg文件中添加手里开发板的信息，包括cpu架构和类型等信息。
   ```

6. **修改顶层目录的makefile，自带交叉编译工具信息：**

   ```shell
   CROSS_COMPLIE ?= gcc-none-linux-gnueabi-
   ```

7. **添加点灯汇编程序，出现问题时用于调试调试缩写问题所指范围**：

   ```txt
   在无法使用串口的情况下，点灯是最靠谱的调试代码，作为嵌入式底层开发人员首先想到的调试办法就是点灯，优先在程序开始的地方(board/....../fs4412/start.S点灯)。
   ```

8. **添加加密引导方式即添加BL1,BL2代码**,（**非必需，因芯片而异**）：

   ```shell
   看开发板芯片芯片手册的引导章节，看有无其他内容要添加，比如Exynos4412需要三星提供的初始引导加密后我们的u-boot才能被引导运行，所以我们需要在uboot源码中添加三星提供的加密处理代码，也即BL1,BL2代码（从官网下载即可）。
   
   CodeSign4SecureBoot/E4412_N.bl1.SCP2G.bin
   CodeSign4SecureBoot/bl2.bin
   
   CodeSign4SecureBoot/all00_padding.bin#填充文件
   
   bl1+bl2+all00_padding.bin+uboot--->u.bin
   
   size(bl1)+size(bl2)+size(all0_padding) + size(uboot) = Booting area
   ```

9. **修改.C，以及start.S文件等进行创新配置**：以此匹配我们的开发板

10. **利用编译脚本配置并编译文件**

11. **测试uboot，观察调试代码是否生效**

### uboot移植——创新

由于**板子不匹配，但芯片是匹配**的，且板子上的不匹配的地方肯定是外设寄存器的设置，对于4412SOC上uart的控制器，寄存器肯定是一样的，所以当芯片一样时，对uart的控制器的引脚初始化肯定一样，要改的是对外设寄存器的控制，且为了防止出错，**创新大都是增加代码，不是修改代码**

故增加的代码修改的文件肯定是**放在boards目录**下的文件的。

#### 移植UART

1. 打开开发板目录下的**板子**初始化文件：

   ```shell
   vi  board/samsung/fs4412/lowlevel_init.S
   ```

2. 初始化临时栈，由于fs4412默认打开看门狗，但origen的代码里没用喂狗操作，故要关闭看门狗。

   ```asm
   	ldr  sp,=0x02060000
   	
   	
   #if 1
   	ldr r0, =0x1002330c
   	ldr r1, [r0]
   	orr r1, r1, #0x300
   	str r1, [r0]
   	ldr r0, =0x11000c08
   	ldr r1, =0x0
   	str r1, [r0]
   /* Clear  MASK_WDT_RESET_REQUEST  */
   	ldr r0, =0x1002040c
   	ldr r1, =0x00
   	str r1, [r0]
   #endif
   ```

3. 初始化UART传输波特率的时钟频率，虽然源文件有串口初始化，但不初始化时钟频率为，UART还是不能正常工作，进对硬件时钟源进行两级分频得到100Mhz。

   ```asm
   	... ...
   	
   	原先就有的对芯片引脚功能设置的代码@明显芯片一样时是匹配的
   	
   	... ...
   	ldr	r0, =0x10030000
   	ldr	r1, =0x666666
   	ldr	r2, =CLK_SRC_PERIL0_OFFSET//选择时钟频率
   	str	r1, [r0, r2]
   	ldr	r1, =0x777777
   	ldr	r2, =CLK_DIV_PERIL0_OFFSET//对时钟进行分频，分到fs4412的100Mhz
   	str	r1, [r0, r2]
   	
   	... ...
   	
   	原先就有的对芯片内部uart控制器的设置的代码@明显芯片一样时是匹配的
   	
   	... ...
   ```

#### 移植网卡

虽然可以通过终端输入命令，但此时的uboot还不能使用ping、tftp等命令，原因在于 命令都是操作网络的，而uboot源码中网卡的相关配置与我们当前的板子不匹配，所以  我们还要对网卡进行移植。

1. 修改网络初始化代码**板级**

   ```shell
   vi  board/samsung/fs4412/fs4412.c
   ```

2. 添加条件编译**板级**

   ```c
   #ifdef CONFIG_DRIVER_DM9000
   	dm9000aep_pre_init();
   #endif
   ```

3. **修改网络配置代码,** `vi include/configs/fs4412.h`,添加一系列宏定义。

#### 移植emmc

因为uboot源码中对EMMC的配置与我们的板子不匹配，这里还需要对EMMC相关的 代码进行修改和配置。

1. 修改初始化代码（**芯片级**），该目录下makefile也要改

   ```shell
   cd  arch/arm/cpu/armv7/exynos/
   ```

2. **修改板级目**录下文件（**板级**）

3. 添加EMMC命令（**命令**）及Makefile

   ```shell
   cd  common/
   ```

4. **修改EMMC配置代码**, `vi include/configs/fs4412.h`,添加一系列宏定义。

#### 5.5.6.4 移植电源管理

1. 修改设备管理目录下的文件

   ```shell
   cd drivers/power/pmic/
   ```

2. **修改配置代码**, `vi include/configs/fs4412.h`,添加一系列宏定义。

3. **修改板级**

4. 修改架构

## 5.6 uboot移植总结

1. 芯片一样，板子不一样时，由于外设会不一样，故都要修改**板级**目录下的文件。
2. 芯片一样，板子不一样时，芯片的引脚配置设置，控制器的设置肯定一样，只是**板子上外设的控制方式**不一样。
3. 外设分复杂外设和简单外设之分，简单外设需要驱动代码，故配置简单外设时，还需要修改**设备配置**文件。`vi include/configs/fs4412.h`
4. 有些需要驱动的复杂外设除了修改配置文件，还可能要修改芯片内部文件甚至是架构文件，更复杂。
5. **由易到难**：简单外设(板级)==>复杂外设(板级+配置)==>更复杂外设(板级+配置+芯片)==>更更复杂外设（板级+配置+芯片+设备管理driver目录）

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