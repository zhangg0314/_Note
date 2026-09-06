# 引导程序

Bootloader是在操作系统运行之前运行的一小段代码，用于将软硬件环境初始化到一个合适的状态，比如初始化时钟，中断，`DRAM`控制器等外设，为操作系统的加载和运行做准备（其本身不是操作系统），再把Linux从Flash拷贝到`DRAM`中，最后再启动Linux内核。Bootloader是启动引导程序的统称，类似于操作系统与Linux的概念，嵌入式Linux常用的Bootloader是U-Boot。

------

# 基本功能

- 初始化软硬件环境，如时钟、内存、串口、存储、网络等。
- 提供调试与维护功能：如环境变量配置、固件升级、网络引导、烧写命令等。
- 引导加载Linux内核，通常是 Linux 内核，并传递启动参数、设备树等必要信息。
- 给Linux内核传参
- 执行用户命令（如U-Boot命令）

------

# U-Boot启动阶段

首先对开发板上的软硬件环境做进一步初始化，然后将Linux内核、设备树(dtb)、根文件系统(rootfs)从外部存储器（或网络）搬移到内存，然后跳转到linux运行。Linux开始运行后先对系统环境做初始化，当系统启动完成后，Linux再从内存中（或网络）挂载根文件系统。

## 1.第一阶段

初始化时钟，关闭看门狗，关中断，关闭MMU，TLB，DCACHE，ICACHE等初始化SDRAM，初始化NAND FLASH等。

[u-boot分析（五）----I/D cache失效|关闭MMU和cache|关闭看门狗 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4222669.html)

```txt
首先，我们为何要关闭mmu？mmu负责从虚拟地址到物理地址之间的转换，
但是我们现在的汇编都是直接操作物理寄存器， 此时如果打开了mmu，而我们并没有有效的TLB，这样cpu可以说是胡乱运行的，
所以我们需要关闭mmu，不需要它转换地址，直接操作寄存器方便快捷。 
然后，再发出灵魂拷问，为何要关闭cache？因为cache和MMU是通过cp15管理的，刚上电的时候，CPU并不能管理他们。
所以上电的时候mmu必须关闭，指令cache可关闭，可不关闭，但数据cache一定要关闭， 否则可能导致刚开始的代码里面，
去取数据的时候，从cache里面取，而这时候RAM中数据还没有cache过来，导致数据预取异常。
```

## 2.第二阶段

​	初始化一个串口，检测系统内存映射，将内核映像和根文件系统映像从Flash上读到DRAM空间中，为内核**设置启动参数**，调用内核。

这种分层设计既保证了对不同硬件的兼容性（通过板级函数定制），又维持了 U-Boot 核心逻辑的统一性（通用代码路径），同时 SPL 与主 U-Boot 共享同一套规则，简化了跨阶段开发。

# U-Boot初始化流程

## 1.总体启动流程
在嵌入式 Linux  系统开发、BSP 适配及新平台 bring-up 过程中，Bootloader 的启动流程往往决定了整个系统移植和硬件支持的复杂度。U-Boot 作为当前主流的开源 Bootloader，其启动流程又分为带 SPL（Secondary Program Loader）与不带 SPL两种模式。不同方案在硬件要求、启动机制、代码结构、调试思路等方面存在明显区别。以 ARM Cortex-A SoC（如 NXP i.MX6/8, Allwinner, Rockchip , STM32MP1 等）为例，完整启动链路如下：

## 2.有 SPL 的启动流程

```txt
BootROM (SoC内部固件) → SPL (精简U-Boot) → U-Boot (完整版) → Linux Kernel → RootFS → User Application
```

## 3.无 SPL 的启动流程

```shell
BootROM (SoC内部固件) → U-Boot (完整版) → Linux Kernel → RootFS → User Application
```

## 4.BootROM 与 SPL 

- **BootROM**
  不可更改，由芯片原厂烧录在 SoC 内部，只负责最基础的硬件初始化和“从外部介质加载 bootloader 到 SRAM/DRAM 并运行”。
- **SPL**
  U-Boot 的精简版，用于解决“内部SRAM容量有限，无法一次加载完整版 U-Boot”时的硬件初始化需求（如 DDR、时钟、简单外设）。
- **完整版 U-Boot**
  具备命令行、丰富外设/协议栈支持，可做升级、调试、启动内核等。

## 5.有无SPL的差异

### 1.有SPL

SOC 上电后，可用的 SRAM 极其有限，无法直接放下大体积的 U-Boot。因此，必须先用极小的 SPL 初始化 DDR，之后将完整版 U-Boot 从外部介质加载到 DDR，再切换 到 U-Boot 运行。因为典型 Cortex-A9/A53/A72 SoC，SRAM 只有 32KB/64KB/128KB，而完整版 U-Boot 动辄数百 KB 甚至上 MB。同时需要复杂 的DRAM 初始化脚本，BootROM 能力有限。

### 2.无 SPL 

SOC 内部 SRAM 或 BootROM 能直接完成 DDR 初始化。BootROM 能直接从存储加载大容量 U-Boot 到 DDR 并运行。U-Boot 镜像尺寸可以接受（如 NOR flash 启动、小型芯片/简单板卡）。早期的 ARM9、ARM11、部分 Cortex-M4/M7，以及 MCU 级应用，内存需求不高。使用 NOR Flash，直接映射到地址空间，无需分段加载。BootROM 足够灵活/支持的 SoC（如部分 Allwinner、Rockchip、早期NXP平台等）。

## 6.启动流程对比

### 1.有 SPL 启动流程详解

1. **BootROM 执行**
   SOC 上电后，BootROM 按固定顺序检测外部存储（SD/eMMC/NAND/NOR/USB等），找到并加载 SPL 到 SRAM。
2. **SPL 阶段**
   初始化必要硬件（主要是时钟/DDR）。极简驱动，仅包含板级初始化和加载U-Boot代码的功能。加载完整 U-Boot 镜像到 DDR。跳转至 U-Boot：SPL 跳转到 DDR 内的 U-Boot 入口，后者接管控制权。

3. **U-Boot 主阶段**
   初始化更丰富的外设。提供 CLI/网络/升级/调试等功能。加载和启动 Linux 内核。操作系统启动。

```c
void board_init_f(ulong dummy)
{
    // 最小化硬件初始化
    arch_cpu_init();
    spl_init();
    dram_init();
    // 读取并加载主 U-Boot
    spl_load_image();
    jump_to_uboot();
}
```

```Kconfig
#（典型 Kconfig/defconfig）
CONFIG_SPL=y
CONFIG_SPL_FRAMEWORK=y
CONFIG_SPL_SERIAL_SUPPORT=y
CONFIG_SPL_DRIVERS_MISC_SUPPORT=y
```

### 2.无SPL 启动流程详解

1. **BootROM 执行**
   SOC 上电后，BootROM 直接从外部存储加载完整 U-Boot 镜像到内存（一般是 DDR）并执行。
2. **U-Boot 阶段**
   U-Boot 自行初始化全部硬件（包含 DDR）。后续流程同上：提供 CLI、升级、内核引导等功能。
3. **操作系统启动**

```c
void board_init_f(ulong dummy)
{
    // 直接初始化所有硬件
    arch_cpu_init();
    dram_init();
    peripheral_init();
    // 进入命令行或直接启动内核
    main_loop();
}
```

.config 文件里无 CONFIG_SPL 相关选项。编译输出只有单个 U-Boot 镜像，无 u-boot-spl 文件。

## 7.开发实践总结

### 1.如何判断需要 SPL

检查 SoC 文档“BootROM 支持的最大加载大小”、“上电后可用的 SRAM/DRAM 初始化流程”。**若 U-Boot 体积超过 BootROM 一次性可加载容量**，必须引入 SPL。若平台/芯片厂商提供的参考 U-Boot 都有 SPL 分段，说明其硬件需要这种启动分阶段。

### 2.编译与适配流程

有 SPL**编译时会生成 u-boot-spl/u-boot-spl.bin，同时 u-boot/u-boot.bin/elf，烧录脚本须注意 SPL 与主镜像分区、拼接位置等。无 SPL只有主镜像直接烧录，启动配置/烧写脚本更简单。

### 3.调试和移植

- 有 SPL 时遇到 DDR 问题，建议优先只修改/调试 SPL，主 U-Boot 尽量保持稳定。
- 无 SPL 时，所有初始化和调试均集中于 U-Boot 代码，调试窗口更大但也更易出错。

```c
Q1：怎么判断我的平台当前用不用 SPL？
查 .config/defconfig 是否有 CONFIG_SPL=y，或者编译输出目录有无 u-boot-spl。
查阅官方 BSP 用户手册，查看推荐的启动分段方案。
    
Q2：什么情况下可以把 SPL 去掉，只用 U-Boot？
硬件初始化极其简单，BootROM 能直接装载主 U-Boot 并执行（如 NOR Flash 映射）。
平台定制需求低，工程团队能确认无后遗症。
    
Q3：SPL 和 U-Boot 如何协作传递信息？
SPL 可通过 RAM、寄存器、中转地址等方式，传递板级信息、环境参数、校验结果。
SPL 失败可直接回退或报错重启，利于早期调试。
    
Q4：如果引导流程挂死，如何排查是 SPL 还是 U-Boot 的问题？
观察串口输出：SPL 阶段 log 是否输出，若 SPL 输出正常则重点排查 U-Boot；否则需定位 SPL 问题。
可以定制 SPL 提示灯/蜂鸣器等硬件动作，辅助区分。
```

# 板级初始化流程

> [!NOTE]
>
> 参考U-Boot源码根目录下的**README**。

## 1.整体流程概述

此流程板卡（boards）预设的启动流程（intended start-up flow”，即硬件板卡正常启动时应遵循的标准步骤。该启动流程对两类程序均适用 ——**SPL（Secondary Program Loader，二级程序加载器）** 和 **U-Boot proper（标准 U-Boot 程序）**，且二者需遵循相同规则（follow the same rules）。

```c
//U-Boot（包括 SPL 和主 U-Boot）的板级初始化遵循统一规则，核心流程为：
架构相关的start.S → lowlevel_init() → board_init_f() → （BSS 清除 + U-Boot镜像重定位） → board_init_r() → main_loop()

//main_loop：uboot-主循环
//起点：从架构 / CPU 专属的start.S开始（如 ARMv7 的 `arch/arm/cpu/armv7/start.S`），这是汇编级别的启动入口。
//核心目标：逐步搭建硬件环境（从最基础的执行条件到完整的内存 / 外设可用），最终进入 U-Boot 主循环处理命令和启动系统。
```

## 2.BSS清除与重定位

在 `board_init_f()` 完成后、`board_init_r()` 执行前，会**自动**进行两项关键操作：

- **BSS 清除**
  清空未初始化的全局变量区域（确保全局变量初始值为 0）。
- **重定位**
  - **SPL**
    若定义 `CONFIG_SPL_STACK_R`，栈和 `global_data` 会被迁移到 `CONFIG_SPL_STACK_R_ADDR` 下方。
  - **主 U-Boot**
    整个 U-Boot 镜像被重定位到内存顶部。

## 3.三大核心函数

### 1.lowlevel_init()

1. **核心作用**
   是启动过程中最早期的初始化函数，仅完成最基础的设置，确保程序能正常执行到`board_init_f()`函数。
2. **限制条件**
   - 不依赖`global_data`（全局数据结构）和 BSS 段（未初始化数据区）
   - 基本没有栈（ARMv7 架构可能有临时栈，但很快会被移除）
   - 禁止初始化 SDRAM（内存）和使用控制台输出
   - 只做让程序能继续执行到`board_init_f()`的最低限度工作
3. **使用场景**
   几乎不需要自定义实现这个函数，通常使用默认实现
4. **执行要求**
   需要正常返回，不能在此处终止执行流程。

### 2.board_init_f()

1. **核心作用**
   为`board_init_r()`函数的运行做准备，重点初始化 SDRAM（同步动态随机存储器）和串行 UART（通用异步收发传输器），使硬件达到可运行后续初始化程序的状态。
2. **运行环境**
   - 可以访问`global_data`（全局数据结构GD，通常通过一个宏定义如 `DECLARE_GLOBAL_DATA_PTR`访问，本质上是一个指向 SRAM 中特定位置的指针，而不属于传统意义上的全局变量）
   - 栈（stack）位于 SRAM（静态随机存储器）中
   - BSS 段（未初始化的全局变量区域）不可用，因此不能使用全局变量或静态变量，只能使用栈变量和`global_data`
3. **无SPL 相关说明**：
   - 会调用`dram_init()`来初始化 DRAM（动态随机存储器）
   - 若在 SPL（Secondary Program Loader，次级程序加载器）中已完成 DRAM 初始化，此函数可什么都不做
4. **SPL 相关说明**：
   - 可根据需要用自定义版本覆盖整个`board_init_f()`函数
   - 在极端情况下可在此处调用`preloader_console_init()`
   - 需完成 SDRAM 初始化以及 UART 工作所需的所有设置
   - 无需清理 BSS 段，这部分工作会由`crt0.S`完成
   - 必须正常返回，不能直接调用`board_init_r()`

### 3.board_init_r()

- **主要作用**
  承担 U-Boot 启动阶段的主要执行逻辑，包含通用初始化代码，最终会跳转到 `main_loop()` 进入命令行交互阶段。
- **运行环境**
  - 可访问 `global_data` 全局数据结构
  - SDRAM 已初始化可用
  - BSS 段已准备好，所有静态 / 全局变量可正常使用
- **非 SPL 场景特性**
      U-Boot 已重定位到SDRAM内存顶部，并从该位置运行。
- **SPL场景特性**：
  - 若定义了 `CONFIG_SPL_STACK_R` 且 `CONFIG_SPL_STACK_R_ADDR` 指向 SDRAM，栈可位于 SDRAM 中
  - 可在此处调用 `preloader_console_init()`（通常通过开启 `CONFIG_SPL_BOARD_INIT` 并实现 `spl_board_init()` 函数来完成）
  - 负责加载 U-Boot 主程序，或在 falcon 模式下直接加载 Linux 内核

## 4.SPL与U-Boot 差异

|      维度      |                             SPL                              |                  主 U-Boot                   |
| :------------: | :----------------------------------------------------------: | :------------------------------------------: |
| **初始化流程** | 遵循相同的三阶段函数（`lowlevel_init` → `board_init_f` → `board_init_r`） |                     同上                     |
|  **代码路径**  |              通常有独立代码路径（`spl/` 目录）               |     主代码路径（`common/`、`board/` 等）     |
|  **核心目标**  |       快速初始化 SDRAM 和串口，加载主 U-Boot 或 Linux        | 完成完整初始化，提供命令行交互，启动操作系统 |
|  **资源限制**  |               体积小，依赖 SRAM 运行，功能精简               |  功能丰富，依赖 SDRAM 运行，可使用全局变量   |
|   **重定位**   |              栈和 `global_data` 可迁移到 SDRAM               |           整个镜像重定位到内存顶部           |

# U-Boot源码结构

## 1.源码下载

[u-boot/u-boot: "Das U-Boot" Source Tree](https://github.com/u-boot/u-boot)

```bash
git clone https://github.com/u-boot/u-boot
```

## 2.uboot源码特点

- 代码结构清晰。
- 支持丰富的处理器与开发板，**易于移植**。
- 支持丰富的用户**命令**，**网络协议**。
- 支持丰富的文件系统（文件系统就是一种管理和访问磁盘的软件）。
- 支持丰富的设备驱动。
- 更新活跃，用户较多，资料丰富，开放源码，较高的稳定性。
- 不具有通用性（不同的处理器，开发板uboot不能通用）。

## 3.uboot源码结构

### 1.芯片平台相关代码

即与CPU架构或开发板硬件相关的源码，硬件的改动对应的代码也要改动。

- **api目录**
  上层接口，仅供外部应用调用使用。

- **arch目录**

  存放各种架构的处理器如ARM，X86等架构的目录。如果只要用ARM，则把其他文件删了就留下ARM也可。ARM里面有个CPU目录，里面有ARM架构体系的各种型号CPU。选择与自己**开发板CPU架构和型号匹配**的文件进行编译即可。

- **board**
  存放不同开发板类型的代码文件。由于哪怕CPU架构和型号一样但开发板不一样，同一代码也不能通用，故还要选择**开发板类型匹配**的文件来进行编译。

### 2.芯片平台无关代码

```shell
- common
   #存放uboot内部自身命令实现的`.c`文件
- COPYING
   #版权信息文件
- CREDITS
   #uboot贡献人员名单
- disk
   #对磁盘的支持文件
- doc，README
   #说明书，说明文档
- drivers
   #驱动
- dts
   #设备树
- fs
   #file system文件系统
```

### 3.api 目录

#### 1.主要用途

​	API 本质让应用能在 U-Boot 阶段（比如设备开机还没启动操作系统时），就能用 U-Boot 的硬件管理能力（读存储、联网、控制设备）。整个设计的核心是 “轻量、灵活”：U-Boot 只做基础转发，应用自己掌控逻辑，还能按需选择是否用辅助工具，适配不同场景的需求。它提供的功能主要包括：

- **控制台操作**
  比如读取和打印字符 (`getc`, `putc`)。
- **系统信息**
  获取平台信息、执行系统复位 (`reset`)。
- **延时与时间**
  进行延时或获取当前时间。
- **环境变量**
  读取、设置或遍历 U-Boot 的环境变量。
- **设备访问**
  枚举、打开、读写存储设备（如 IDE, SCSI, USB）和网络设备。

#### 2.API设计标准

这部分是设计这个 API 时定下的标准。

|                           设计前提                           |                             解释                             |
| :----------------------------------------------------------: | :----------------------------------------------------------: |
|                 由单一入口（syscall）到 API                  |  syscall = 系统调用，就是 “应用向底层系统要服务的请求通道”   |
| 当前 syscall 是 U-Boot 的 C 函数，未来可能升级为 “机器异常陷阱” |    机器异常陷阱，一种硬件级的调用方式，更稳定、防误操作。    |
| 消费者应用（用 API 的程序）自己准备 “上下文”（调用号 + 参数） |              调用syscall 需要传调用号和相关参数              |
| syscall 会把请求 “分发” 到 U-Boot 的现有功能区（网络 / 存储等） |                   就是类似于C++的多态重载                    |
|             应用通过 “找签名” 确认 API 是否可用              | U-Boot 会在设备的某个固定内存地址（比如 0x10000000）留下一个 “标记”（比如一串特定的二进制代码，叫 “签名”）；应用启动后，会去这个地址找这个标记，找到就说明 API 能用，没找到就说明设备没装这个 API。 |
|   U-Boot 的 API 部分 “轻量不打扰”，尽量让应用自己处理逻辑    | U-Boot 的 API 只做 “最基础的转发 / 执行”，不保存状态，应用下次调用时，得重新给参数。 |
|                    可选功能（CONFIG_API）                    | 这个 API 不是 U-Boot 的 “必选项”—— 编译 U-Boot 时，选了 “CONFIG_API” 这个开关，设备就带这个 API；没选，设备里的 U-Boot 就没有这个功能，应用也没法调用。 |

#### 3.API的具体功能

这部分是应用通过 API 能让 U-Boot 做哪些事，分 5 大类：

##### 1.Console 相关

- getc：读取用户输入的一个字符。
- putc：往屏幕 / 串口输出一个字符。
- tstc：检查有没有用户输入（比如看用户有没有按键盘，有就返回 “有”，没有就返回 “没有”）。

##### 2.System 相关

- reset：重启设备（和按物理重启键效果一样）。
- platform info：读取设备的硬件参数（比如 CPU 型号、设备型号、内存大小）。

##### 3.Time 相关

- delay：延时等待。
- current：读取设备 开机到现在的时间。

##### 4.Envvars 相关

- enumerate all：把 U-Boot 里存的所有配置参数（比如 IP、启动路径）都列出来。
- get：查某个参数的具体值（比如查 IP 地址是 192.168.1.100）。
- set：改某个参数的值（比如把 IP 改成 192.168.1.101）。

##### 5.Devices 相关

- enumerate all：列出所有设备，比如列出 “SD 卡、USB 硬盘、网卡（eth0）。
- open：打开设备。
- close：关闭设备。
- read：读设备。
- write：写设备。

#### 4.API的整体结构

##### 1.Core API

U-Boot 内部的 “核心组件”，是整个 API 的 “中枢”—— 唯一的 syscall 入口就是它实现的，所有应用的请求都要经过它分发。只做分发，不自己处理具体业务（比如不自己读 SD 卡，只把 “读 SD 卡” 的请求转给存储模块）。

##### 2.Glue

Glue字面意思是胶水，负责把 应用和Core API粘起来，让两者能对接。

|          组件          |                       功能                       |                             举例                             |
| :--------------------: | :----------------------------------------------: | :----------------------------------------------------------: |
| 必选部分：消费者侧入口 |      应用的对接接口，让应用调用 Core API。       | 比如在应用代码里写`glue_syscall(调用号, 参数)`，这个函数就是必选的glue层入口，负责把请求传给 Core API。 |
| 可选部分：辅助包装函数 | 把 “复杂的 syscall 调用步骤” 包装成 “简单的函数” | 比如把 “准备 putc 的调用号（0x01）+ 字符参数（'A'）+ 触发 syscall” 这三步，包装成一个`uart_putc('A')`函数，应用直接写`uart_putc('A')`就能在串口输出 'A'。 |

##### 3.Consumer Application

是 API 的 “使用者”，通过两种方式调用 API：

1. 直接调用
   自己准备调用号 + 参数，通过胶水层的必选入口触发 syscall；
2. 间接调用
   用胶水层的可选辅助函数（比如`uart_putc`），更简单。

例子：应用要输出 “Hello”：

- 直接调用
  自己写`glue_syscall(0x01, 'H')`→`glue_syscall(0x01, 'e')`→…→依次输出每个字符；
- 间接调用
  如果胶水层有`console_print("Hello")`的辅助函数，直接写`console_print("Hello")`就行，函数内部会自动处理调用号和参数。

# U-Boot配置编译

理想状态下，即处理器架构型号，以及开发板类型，uboot源码中都有，那么不需要我们去修改代码来匹配我们自己的开发板，可以直接进行配置编译。

## 1.U-Boot配置

1. **指定当前使用的硬件平台和CPU架构**

   ```shell
   make <board_name>_config 
   
   #注1：<board_name>为当前使用的开发板的名字
   
   #注2：执行该命令的前提是uboot源码支持该开发板
   
   #注3：该命令必须在uboot源码的顶层目录下执行
   
   #example
   make origen_config
   ```
   
2. **指定编译uboot源码使用的编译器**

   ```shell
   #在uboot源码顶层目录下的Makefile中指定(CROSS_COMPILE变量)
   ifeq (arm,arm)
   	CROSS_COMPILE ?= arm-none-linux-gnueabi-
   endif
   #example
   CROSS_COMPILE ?= arm-none-linux-gnueabi-
   ```

## 2.U-Boot编译

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

## 3.U-Boot移植

以网上看到的fs4412教程为例：要先修改一系列文件来适配fs4412，才能进行配置编译。

1. **添加Board信息**
   因为uboot源码并不支持我们的开发板，这里我们需要从源码支持的开发板中找一个硬件与我们最类似的，在其基础上进行修改，这里我们参考的是samsung公司的origen。

   ```txt
   直接将origen目录复制一份改目录名为fs4412，修改目录中的.c和.h的文件名，因为修改了文件名，所以对应的Makefile内容也要修改，然后再修改.h头文件中的内容中打印开发板名字的宏定义。最后再在boards.cfg中添加fs4412的相关信息。
   ```
   
2. **添加三星加密引导方式**（因芯片而异，不需要掌握）

   考虑芯片启动的安全性，Exynos4412需要三星提供的初始引导加密后我们的u-boot才能被引导运行，所以我们需要在uboot源码中添加三星提供的加密处理代码。添加完后，又因为添加的加密文件也要编译，所以对应的Makefile也要修改。

   如瑞芯微的需要添加idbloader.img（TPL/SPL）、trust.img(ATF/TEE)等引导启动。[Boot option - Rockchip open source Document](https://opensource.rock-chips.com/wiki_Boot_option)

3. **添加调试代码（点灯法）**

   很多时候我们不确定uboot是否已经在板子上运行，所以我们在uboot源码中添加一段代码使板子上的LED点亮，这样如果看到LED亮的话就表示uboot已经在运行了。打开uboot启动后的第一段代码

   ```shell
   vi arch/arm/cpu/armv7/start.S
   ```
   
    在第134行后添加如下代码（即点亮LED2），然后保存退出

   ```asm
   ldr r0, =0x11000c40
   
   ldr r1, [r0]
   
   bic r1, r1, #0xf0000000
   
   orr r1, r1, #0x10000000
   
   str r1, [r0]
   ```
   
4. **添加编译脚本**

   使用make命令编译时只链接uboot源码中的相关代码，而我们添加的初始引导加密的代码不会被连接到u-boot.bin中，所以这里我们自己编写编译脚本build.sh，这个脚本中除了对uboot源码进行配置和编译外还将初始引导加密代码链接到了u-boot.bin上，最终生成一个完成的uboot镜像u-boot-fs4412.bin。

5. **实现串口输出**

   虽然uboot已经能在开发板上加载运行，但是此时的uboot还不能在终端上打印信息，原因在于uboot源码中对UART的配置与我们实际的硬件不匹配。

## 4.移植步骤总结

1. **明确需求**
   把Linux内核正常运行起来，先要移植uboot。

2. **准备工作**
   先去官网（芯片厂商官网、Github等）下载U-Boot源码压缩包，然后解压。

3. **确认芯片是否支持**
   确认芯片架构和cpu类型（arch/arm/cpu/armv7）

   ```shell
   #支持（一般芯片厂商如瑞芯微有github仓库支持所有型号的）：
   	goto 4
   #不支持
   	找最新的uboot源码/找芯片厂家要（SDK）
   ```

4. **确认板子是否支持**

   ```shell
   #支持
   	直接配置编译
   #不支持
   	找最新的uboot源码/找板子厂家要/自己移植 goto 5
   ```

5. **代码移植**

   ```shell
   #1.从源码支持的开发板中找一个硬件与我们最类似的，在其基础上进行修改，比如我们参考的是samsung公司的origen
   1)把board目录下的origen目录直接复制一份并重命名为fs4412,然后重命名该目录里的.c文件，由于修改了文件名，故此时同目录下的makefile文件内容要修改。
   2)进入include/configs目录，复制一份origen.h头文件并重命名为fs4412.h，并修改.h头文件里面内容，把origen改为fs4412
   3)在顶层目录的board.cfg文件中添加手里开发板的信息，包括cpu架构和类型等信息。
   ```
   
6. **修改顶层目录的makefile，自带交叉编译工具信息**

   ```shell
   CROSS_COMPLIE ?= gcc-none-linux-gnueabi-
   ```

7. **添加点灯汇编程序，出现问题时用于调试调试缩写问题所指范围**：

   ```shell
   #在无法使用串口的情况下，点灯是最靠谱的调试代码，作为嵌入式底层开发人员首先想到的调试办法就是点灯，优先在程序开始的地方(board/....../fs4412/start.S点灯)。
   ```

8. **添加加密引导方式即添加BL1,BL2代码**（**非必需，因芯片而异**）：

   ```shell
   #看开发板芯片芯片手册的引导章节，看有无其他内容要添加，比如Exynos4412需要三星提供的初始引导加密后我们的u-boot才能被引导运行，所以我们需要在uboot源码中添加三星提供的加密处理代码，也即BL1,BL2代码（从官网下载即可）。
   
   CodeSign4SecureBoot/E4412_N.bl1.SCP2G.bin
   CodeSign4SecureBoot/bl2.bin
   CodeSign4SecureBoot/all00_padding.bin#填充文件
   bl1+bl2+all00_padding.bin+uboot--->u.bin
   size(bl1)+size(bl2)+size(all0_padding) + size(uboot) = Booting area
   ```
   
9. **修改.C，以及start.S文件等进行创新配置**：以此匹配我们的开发板

10. **利用编译脚本配置并编译文件**

11. **测试uboot，观察调试代码是否生效**

## 5.U-Boot板级移植

由于**板子不匹配，但芯片是匹配**的，且板子上的不匹配的地方肯定是外设寄存器的设置，对于4412SOC上uart的控制器，寄存器肯定是一样的，所以当芯片一样时，对uart的控制器的引脚初始化肯定一样，要改的是对外设寄存器的控制，且为了防止出错，板级大都是增加代码，不是修改代码。故增加的代码修改的文件大都是**放在boards目录**下的文件的。

### 1.移植UART

1. 打开开发板目录下的板子初始化文件：

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
   
3. 初始化UART传输波特率的时钟频率，虽然源文件有串口初始化，但不初始化时钟频率，UART还是不能正常工作，进对硬件时钟源进行两级分频得到100Mhz。

   ```asm
   	... ...
   	
   	#原先就有的对芯片引脚功能设置的代码@明显芯片一样时是匹配的
   	
   	... ...
   	ldr	r0, =0x10030000
   	ldr	r1, =0x666666
   	ldr	r2, =CLK_SRC_PERIL0_OFFSET//选择时钟频率
   	str	r1, [r0, r2]
   	ldr	r1, =0x777777
   	ldr	r2, =CLK_DIV_PERIL0_OFFSET//对时钟进行分频，分到fs4412的100Mhz
   	str	r1, [r0, r2]
   	
   	... ...
   	
   	#原先就有的对芯片内部uart控制器的设置的代码@明显芯片一样时是匹配的
   	
   	... ...
   ```

### 2.移植网卡

虽然可以通过终端输入命令，但此时的uboot还不能使用ping、tftp等命令，原因在于命令都是操作网络的，而uboot源码中网卡的相关配置与我们当前的板子不匹配，所以我们还要对网卡进行移植。

1. 修改网络初始化代码

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

### 3.移植emmc

因为uboot源码中对EMMC的配置与我们的板子不匹配，这里还需要对EMMC相关的代码进行修改和配置。

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

### 4.移植电源管理

1. 修改设备管理目录下的文件

   ```shell
   cd drivers/power/pmic/
   ```

2. **修改配置代码**, `vi include/configs/fs4412.h`,添加一系列宏定义。

3. **修改板级**

4. 修改架构

## 6.U-Boot移植总结

1. 芯片一样，板子不一样时，由于外设会不一样，故都要修改**板级**目录下的文件。
2. 芯片一样，板子不一样时，芯片的引脚配置设置，控制器的设置肯定一样，只是**板子上外设的控制方式**不一样。
3. 外设分复杂外设和简单外设之分，简单外设需要驱动代码，故配置简单外设时，还需要修改**设备配置**文件。`vi include/configs/fs4412.h`
4. 有些需要驱动的复杂外设除了修改配置文件，还可能要修改芯片内部文件甚至是架构文件，更复杂。
5. **由易到难**：简单外设(板级)==>复杂外设(板级+配置)==>更复杂外设(板级+配置+芯片)==>更更复杂外设（板级+配置+芯片+设备管理driver目录）

# U-Boot编译产物

`u-boot`（ELF，类似 vmlinux）是所有二进制的源头。

```shell
u-boot (ELF)
 ├─ objcopy → u-boot-nodtb.bin    #纯代码，无 dtb
 ├─ +dtb    → u-boot-dtb.bin
 │     └─ 复制 → u-boot.bin        #常见可烧写文件
 ├─ +image头 → u-boot.img / u-boot-dtb.img
 └─ 链接器副产：.map / .sym / System.map
```

## 1.通用固定产物

| 文件                         | 说明                                        |
| ---------------------------- | ------------------------------------------- |
| `u-boot`                     | ELF，带符号，调试用                         |
| `u-boot.bin`                 | 裸二进制                                    |
| `u-boot-nodtb.bin`           | 无 dtb 的 bin                               |
| `u-boot.dtb`                 | U-Boot 自己的设备树（含 dm、安全启动公钥）  |
| `u-boot-dtb.bin`             | bin + dtb 合并                              |
| `u-boot.img`                 | 加 image 头封装，SPL拉U-Boot proper 用      |
| `u-boot.itb`                 | FIT 镜像，可含 uboot + OP-TEE + dtb + 签名  |
| `u-boot.map`                 | 链接内存布局                                |
| `u-boot.sym`                 | 符号表                                      |
| `System.map`                 | 符号地址映射                                |
| `u-boot.srec` / `u-boot.hex` | Motorola S-record / Intel hex，老式烧写格式 |

## 2.平台相关产物

BROM（片内启动 ROM）决定：读多大、从哪个 offset、要不要专用头/校验。产物名由这些决定。

| 平台            | 产物                                                         | 特点                                                         |
| --------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| Rockchip        | `idbloader.img`（TPL+SPL 加 RK 头，`mkimage -T rksd/rkspi`）、   `u-boot-rockchip.bin`、老 SDK `uboot.img`（`tools/loaderimage --pack`）+ `trust.img`(BL31) | BROM 最多读 32KB，按 512B 块补零；SD 按分区 offset 写（idbloader ~32K，itb ~8M） |
| Allwinner       | `u-boot-sunxi-with-spl.bin`（SPL+uboot+dtb 拼一个文件）      | `mksunxiboot` 加 BROM 认的签名/校验；`dd seek=8` 写 SD       |
| TI (AM335x 等)  | `MLO`（SPL 加 TI 头，MMC/SD 引导固定名）、NAND 引导叫 SPL、`u-boot.img` |                                                              |
| NXP i.MX6/7     | `u-boot.imx`（`mkimage -T imximage` 加 IVT+DCD 头，前 0xC00 pad）、`u-boot-with-spl.imx` | i.MX8 用 binman 出 `flash.bin`/`imx-boot*`                   |
| MXS (i.MX23/28) | `u-boot.ais`、`u-boot.sb`（签名）                            | elftosb 生成                                                 |
| STM32MP         | `u-boot.stm32`（FSBL 头+uboot proper）                       |                                                              |
| Xilinx Zynq     | `boot.bin`                                                   | bootgen 打包                                                 |
| Marvell Armada  | `u-boot-spl.kwb` / `u-boot-with-spl.bin`                     | `mkimage -T kwbimage`                                        |
| Intel x86       | `u-boot.rom`                                                 | binman 拼，含 FSP/ifwi                                       |
| NVIDIA Tegra    | BCT 头 + bootloader 按 512B 块                               |                                                              |

- `.bin` = 裸数据
- `.img` = 带容器头、可整块 dd 的分区镜像
- `.imx`/`.ais`/`.kwb`/`.stm32`/`.rom`/`.sb` = 该家 BROM 专用头格式

## 3.能否直接固化

判断标准一条: 是不是BROM或上一级loader期望的最终形态。是的话可直接烧录固化; 否则就是中间件还得加工。

### 1.中间产物

| 文件                                                         | 原因                                     |
| ------------------------------------------------------------ | ---------------------------------------- |
| `u-boot`（ELF）                                              | 带符号段/调试信息，非纯执行码            |
| `u-boot-nodtb.bin`                                           | 缺 dtb                                   |
| `u-boot.dtb`                                                 | 只是数据                                 |
| `spl/u-boot-spl`、`spl/u-boot-spl-nodtb.bin`、`spl/u-boot-spl.dtb` | SPL 没加平台头前 BROM 不认               |
| `*.map` / `*.sym` / `System.map`                             | 调试符号，非镜像                         |
| `.its`                                                       | 编译输入                                 |
| `u-boot.srec` / `.hex`                                       | 老式传输格式，靠工具烧，不放 flash直接跑 |

### 2.半成品

| 文件                                            | 缺失部分                                                     |
| ----------------------------------------------- | ------------------------------------------------------------ |
| `spl/u-boot-spl.bin` / `spl/u-boot-spl-dtb.bin` | 加平台 BROM 头：TI→`MLO`、sunxi→`mksunxiboot`、RK→并入 `idbloader.img`、i.MX→加 IVT |
| `u-boot-dtb.bin`                                | 裸烧可以（=u-boot.bin），也常再包成 `.img` / `uboot.img` / `.imx` |
| `u-boot.bin`                                    | 有的平台（NOR）直接烧，有的再转                              |

### 3.可直接固化

| 文件                                 | 烧写位置                              |
| ------------------------------------ | ------------------------------------- |
| `u-boot.img` / `u-boot-dtb.img`      | uboot 分区或 mmc 固定扇区             |
| `u-boot.itb`                         | 有对应的分区或者offset                |
| `u-boot-sunxi-with-spl.bin`（SUNXI） | 单文件 `dd seek=8` 写 SD              |
| `idbloader.img`（RK）                | loader1 分区 `dd seek=64`             |
| `u-boot-rockchip.bin`                | idbloader+itb 拼好，写 eMMC 起始/量产 |
| `MLO`                                | SD/MMC 固定位置                       |
| `trust.img`（RK，BL31）              | trust 分区                            |
| `u-boot.imx` / `u-boot-with-spl.imx` | 存储对应 offset，IVT 给 BROM 读       |
| `u-boot.stm32`                       | FSBL 分区                             |
| `boot.bin`（Zynq）                   | 启动介质                              |
| `u-boot.rom`（x86）                  | SPI flash                             |
| `uboot.img`（RK 老 SDK）             | uboot 分区                            |

### 4.编译烧录总结

同后缀跨平台并无统一答案。比如`u-boot.bin` 在 A 板直接烧 NOR、而在 B 板要先包 IVT。所有需要看具体板子的 `doc/README.<vendor>` 或烧录脚本，脚本里列出啥、丢哪个 offset、加什么头，那个才是该板真正可直接固化的。

# U-Boot加载启动

## 1.SD卡加载U-Boot

### 1.SD卡存储结构

SD卡的存储以**扇区为单位**，每个扇区的大小为**512Byte**。其中零扇区存储分区表（即分区信息），后续的扇区可自行分区和格式化。若选择SD卡启动，处理器上电后**从第一个扇区开始**将其中的内容搬移到内存，所以我们把U-Boot放到从第一个扇区开始之后的空间， 之后的空间根据个人需求可进行分区和格式化。

### 2.刷卡步骤详解

1. 执行如下命令，制作一个 1M 的空镜像（用于擦除 SD 卡中原有的数据）

   ```shell
   dd  参数 对象 #Data Duplicator（数据复制器）
   sudo dd if=/dev/zero of=clear.bin count=2048
   
   #dd就是用于制作文件，
   #if == inputfile， /dev/zero根目录下系统自带的文件，
   #of = outfile， clear.bin制作的可自定义的文件名
   #bs == 块大小（默认512字节），count == 块个数，2048 * 512 = 1M
   ```

2. 由于刷卡时，刷写程序默认是从第0块区开始刷，但uboot需要放到第1块区，而一块区大小为512字节，故需要先往里面刷一块512字节的程序来填充第0块。故在终端输入如下命令，制作一个 512 字节的空镜像。

   ```shell
   sudo  dd  if=/dev/zero  of=zero.bin  count=1
   ```

3. 执行如下命令，将uboot追加到zero.bin之后合并生成win-u-boot-fs4412.bin。

   ```shell
   cat  zero.bin  u-boot-fs4412.bin  > win-u-boot-fs4412.bin
   ```

4. 把生成的win-u-boot-fs4412.bin和clear.bin拷贝到windows下。

5. 将SD卡插入到电脑上（卡槽/USB读卡器均可），使其在windows下识别（若不识别，可能是已经在ubuntu中识别了，在可移动设备中将其断开即可）。

6. 利用`Win32DiskImager`把**clear.bin**和**win-u-boot-fs4412.bin**依次刷入SD卡。

## 2.eMMC加载uboot

1. 通过`Secure CRT`先利用TFTP把uboot加载到内存。
2. emmc open 0 ，打开EMMC引导分区
3. mmc write 0 0x41000000 0x0 0x800，把uboot刷到EMMC的0扇区。**注意uboot刷到SD卡，刷到第1块，而刷到EMMC，刷到第0块**
4. emmc close 0，关闭EMMC引导分区

## 3.RK3506刷机加载

# U-Boot命令行

`uboot`环境变量`bootargs`是uboot传递给Linux的自启动参数，用于告诉Linux内核接下来该怎么初始化，怎么运行等。

```shell
setenv bootargs 
root=/dev/nfs
nfsroot=192.168.0.200:/opt/4412/rootfs/ 
rw 
console=ttySAC2,115200 
init=/linuxrc 
ip=192.168.0.102 
#root 告诉linux内核通过什么方式挂载根文件系统
#nfsroot 根文件系统所处主机以及所处路径
#rw 开发板对文件系统里文件的读写权限
#console 告诉linux打印信息通过哪个串口打印，并设置打印波特率
#init 告诉linux内核init启动的第一个进程在哪里,负责初始化系统设置、加载驱动程序、设置文件系统和启动系统服务等工作。
#ip 告诉linux内核自己的ip地址是多少，设置好后可以通过ifconfig查看。
```

# U-Boot源码结构

## 1. 源码下载

[u-boot/u-boot: "Das U-Boot" Source Tree](https://github.com/u-boot/u-boot)

```bash
git clone https://github.com/u-boot/u-boot
```

## 2. uboot源码特点

- 代码结构清晰。
- 支持丰富的处理器与开发板，**易于移植**。
- 支持丰富的用户**命令**，**网络协议**。
- 支持丰富的文件系统（文件系统就是一种管理和访问磁盘的软件）。
- 支持丰富的设备驱动。
- 更新活跃，用户较多，资料丰富，开放源码，较高的稳定性。
- 不具有通用性（不同的处理器，开发板uboot不能通用）。

## 3. uboot源码结构

### 1.芯片平台相关代码

即与CPU架构或开发板硬件相关的源码，硬件的改动对应的代码也要改动。

- **api目录**
  上层接口，仅供外部应用调用使用。

- **arch目录**

  存放各种架构的处理器如ARM，X86等架构的目录。如果只要用ARM，则把其他文件删了就留下ARM也可。ARM里面有个CPU目录，里面有ARM架构体系的各种型号CPU。选择与自己**开发板CPU架构和型号匹配**的文件进行编译即可。

- **board**
  存放不同开发板类型的代码文件。由于哪怕CPU架构和型号一样但开发板不一样，同一代码也不能通用，故还要选择**开发板类型匹配**的文件来进行编译。

### 2.芯片平台无关代码

```shell
- common
   #存放uboot内部自身命令实现的`.c`文件
- COPYING
   #版权信息文件
- CREDITS
   #uboot贡献人员名单
- disk
   #对磁盘的支持文件
- doc，README
   #说明书，说明文档
- drivers
   #驱动
- dts
   #设备树
- fs
   #file system文件系统
```

## 4.api 目录

### 1.主要用途

**U-Boot** 是嵌入式设备（比如路由器、智能机顶盒、工业控制板）的 “开机引导程序”—— 就像电脑开机先跑 BIOS，U-Boot 负责启动设备里的真正操作系统（如 Linux），同时还能管理硬件（存储、网络）、处理基础操作。这个 API 的目标是让独立的应用程序（比如在 U-Boot 环境下运行的测试程序或工具）能够调用 U-Boot 内部的核心功能。它提供的功能主要包括：

- **控制台操作**：比如读取和打印字符 (`getc`, `putc`)。
- **系统信息**：获取平台信息、执行系统复位 (`reset`)。
- **延时与时间**：进行延时或获取当前时间。
- **环境变量**：读取、设置或遍历 U-Boot 的环境变量。
- **设备访问**：枚举、打开、读写存储设备（如 IDE, SCSI, USB）和网络设备。

### 2.API设计标准

这部分是设计这个 API 时定下的标准。

|                           设计前提                           |                           通俗解释                           |
| :----------------------------------------------------------: | :----------------------------------------------------------: |
|                 由单一入口（syscall）到 API                  | API 只有一个 “大门”，外部应用要调用 U-Boot 的功能，必须从这个门进，不能走侧门 / 后门。 （syscall = 系统调用，就是 “应用向底层系统要服务的请求通道”） |
| 当前 syscall 是 U-Boot 的 C 函数，未来可能升级为 “机器异常陷阱” | 现在这个 “大门” 是临时的 “木板门”（C 函数），能满足基础需求；等验证好用后，会换成更安全的 “防盗铁门”（机器异常陷阱，一种硬件级的调用方式，更稳定、防误操作）。 |
| 消费者应用（用 API 的程序）自己准备 “上下文”（调用号 + 参数） | “上下文” 就是 “我要干啥 + 用啥条件”—— 应用想让 U-Boot 做事，必须自己说清 “做什么”（调用号）和 “怎么做 / 用什么做”（参数），U-Boot 不帮你想。 |
| syscall 会把请求 “分发” 到 U-Boot 的现有功能区（网络 / 存储等） | 应用的请求进了 “大门” 后，U-Boot 会把请求转给对应 “部门” 处理 —— 比如要读 SD 卡就转给 “存储部门”，要联网就转给 “网络部门”。 |
|             应用通过 “找签名” 确认 API 是否可用              | U-Boot 会在设备的某个固定内存地址（比如 0x10000000）留下一个 “标记”（比如一串特定的二进制代码，叫 “签名”）；应用启动后，会去这个地址找这个标记，找到就说明 API 能用，没找到就说明设备没装这个 API。 |
|   U-Boot 的 API 部分 “轻量不打扰”，尽量让应用自己处理逻辑    | U-Boot 的 API 只做 “最基础的转发 / 执行”，不记事儿、不搞复杂逻辑 —— 比如不保存应用上次做了啥（不存状态），应用下次调用时，得重新告诉 U-Boot “当前情况”（给提示 / 参数）。 |
|                    可选功能（CONFIG_API）                    | 这个 API 不是 U-Boot 的 “必选项”—— 编译 U-Boot 时，选了 “CONFIG_API” 这个开关，设备就带这个 API；没选，设备里的 U-Boot 就没有这个功能，应用也没法调用。 |

## 3. API能实现的具体功能

这部分是 API 的 “服务菜单”—— 应用通过 API 能让 U-Boot 做哪些事，分 5 大类：

### 11.1 Console 相关（控制台操作）

- 术语解释：“Console” 是设备的 “人机交互接口”（比如串口、设备自带的小屏幕 + 键盘）；相关功能都是 “和用户交互的基础操作”。
  - getc：“拿字符”—— 读取用户输入的一个字符（比如按键盘的 “A” 键，getc 就把 “A” 读出来）。
  - putc：“放字符”—— 往屏幕 / 串口输出一个字符（比如想显示 “Hi”，就用 putc 依次输出 “H” 和 “i”）。
  - tstc：“查字符”—— 检查有没有用户输入（比如看用户有没有按键盘，有就返回 “有”，没有就返回 “没有”）。
- 具体例子：开发一个 “嵌入式终端 APP”（比如设备调试用的命令行工具）：
  - 想显示 “请输入指令：”，就用`putc`逐个输出字符，最终在屏幕上显示这句话；
  - 然后用`tstc`循环检查 “用户有没有按键盘”，一旦检测到有输入，就用`getc`把用户按的键（比如 “reboot”）读出来，再执行对应的操作。

### 11.2 System 相关（系统操作）

- 术语解释：针对设备整体的操作，核心是 “控制设备状态” 和 “获取设备信息”。
  - reset：“重启设备”—— 让设备断电重启（和按物理重启键效果一样）。
  - platform info：“获取平台信息”—— 读取设备的硬件参数（比如 CPU 型号、设备型号、内存大小）。
- 具体例子：开发一个 “设备诊断 APP”：
  - 如果检测到系统文件损坏、无法修复，就调用`reset`让设备重启，尝试恢复正常；
  - 如果想在 APP 界面显示 “设备型号：工业控制板 X300，CPU：ARM A53”，就调用`platform info`获取这些参数，再用 console 功能显示出来。

### 11.3 Time 相关（时间操作）

- 术语解释：处理 “时间延迟” 和 “时间查询”，核心是 “控制执行节奏” 和 “记录时间点”。
  - delay：“延时等待”—— 让程序暂停指定时间（比如延时 1000 毫秒 = 1 秒，期间不做任何操作）。
  - current：“获取当前时间”—— 读取设备 “开机到现在的时间”（比如开机后第 5 分 20 秒）。
- 具体例子：开发一个 “LED 控制 APP”（让设备上的 LED 灯闪烁）：
  - 调用`putc`输出 “LED 开始闪烁”，然后执行 “开灯→调用 delay (1000)（等 1 秒）→关灯→调用 delay (1000)（等 1 秒）” 的循环，实现 LED 每秒闪一次；
  - 如果想记录 “LED 闪烁了多少次”，就每次闪烁时调用`current`获取当前开机时间，记下来（比如 “第 3 秒时第 1 次闪烁，第 5 秒时第 2 次闪烁”）。

### 11.4 Env vars 相关（环境变量操作）

- 术语解释：“Env vars”（环境变量）是 U-Boot 里存的 “配置参数”（比如设备的 IP 地址、启动时读取系统镜像的路径、硬件初始化参数），相当于 U-Boot 的 “小记事本”。
  - enumerate all：“列出所有变量”—— 把 U-Boot 里存的所有配置参数（比如 IP、启动路径）都列出来。
  - get：“读取变量值”—— 查某个参数的具体值（比如查 IP 地址是 192.168.1.100）。
  - set：“修改变量值”—— 改某个参数的值（比如把 IP 改成 192.168.1.101）。
- 具体例子：开发一个 “网络配置 APP”（给设备改 IP）：
  - 先调用`get`读取当前 IP（比如得到 “192.168.1.100”），用 console 显示给用户；
  - 用户输入新 IP“192.168.1.101” 后，调用`set`把 IP 改成新值；
  - 如果用户想确认所有网络配置，就调用`enumerate all`，列出 “IP=192.168.1.101、网关 = 192.168.1.1、子网掩码 = 255.255.255.0” 等所有参数。

### 11.5 Devices 相关（设备操作）

- 术语解释：管理设备上的硬件（目前支持 “存储设备” 和 “网络设备”），核心是 “操作硬件的标准流程”（找设备→打开→读写→关闭）。
  - enumerate all：“列出所有设备”—— 比如列出 “SD 卡、USB 硬盘、网卡（eth0）”。
  - open：“打开设备”—— 相当于 “占用设备”，告诉 U-Boot“我要用地这个硬件了，别给别人用”。
  - close：“关闭设备”—— 用完后 “释放设备”，让其他程序能用地。
  - read：“读设备”—— 从硬件里拿数据（比如从 SD 卡读一个文件）。
  - write：“写设备”—— 往硬件里存数据（比如往 SD 卡写一个日志文件）。
- 具体例子：开发一个 “SD 卡文件读取 APP”（从 SD 卡读配置文件）：
  1. 调用`enumerate all`，列出所有存储设备，找到 “SD 卡（设备名：mmc0）”；
  2. 调用`open(mmc0)`，打开 SD 卡（告诉 U-Boot “我要用 SD 卡了”）；
  3. 调用`read(mmc0, 文件名：config.txt, 读取地址：0x20000000)`，把 SD 卡里的`config.txt`文件读到内存的 0x20000000 地址；
  4. 读完后调用`close(mmc0)`，释放 SD 卡（别人现在能用地了）；
  5. 最后用 console 显示`config.txt`里的内容（比如 “设备最大亮度：80%”）。

## 12. API 的整体结构

这部分是 API 的 “组件分工”—— 就像一个公司的 “部门设置”，每个组件负责不同的事，协同工作。

### 12.1 Core API（核心 API，必选）

- **术语解释**：U-Boot 内部的 “核心组件”，是整个 API 的 “中枢”—— 唯一的 syscall 入口就是它实现的，所有应用的请求都要经过它分发。
- 关键特点：
  - 必须有（U-Boot 只要带这个 API，就不能少它）；
  - 只做 “分发”，不自己处理具体业务（比如不自己读 SD 卡，只把 “读 SD 卡” 的请求转给存储模块）。

### 12.2 Glue（胶水层，分 “必选” 和 “可选”）

“Glue” 字面意思是 “胶水”—— 负责把 “应用” 和 “Core API” 粘起来，让两者能对接。

| 胶水层组件             | 功能解释                                                     | 通俗类比                                                     | 例子                                                         |
| ---------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 必选部分：消费者侧入口 | 应用这边的 “对接接口”—— 相当于给应用装了一个 “能连 U-Boot 的插头”，没有这个插头，应用根本没法调用 Core API。 | 手机充电需要 “手机充电口”（应用侧入口），没有这个口，充电器（Core API）插不进去，没法充电。 | 应用要调用 syscall，必须通过这个入口 “触发请求”—— 比如在应用代码里写`glue_syscall(调用号, 参数)`，这个函数就是必选的胶水层入口，负责把请求传给 Core API。 |
| 可选部分：辅助包装函数 | 把 “复杂的 syscall 调用步骤” 包装成 “简单的函数”—— 应用不用自己准备调用号、参数、触发 syscall，直接调用包装好的函数就行，更友好。 | 相当于把 “手动调光圈、快门、ISO”（直接调用 syscall）包装成 “按一下拍照键”（辅助函数），普通人不用懂专业操作，也能拍照。 | 比如把 “准备 putc 的调用号（0x01）+ 字符参数（'A'）+ 触发 syscall” 这三步，包装成一个`uart_putc('A')`函数，应用直接写`uart_putc('A')`就能在串口输出 'A'，不用自己搞复杂步骤。 |

### 12.3 Consumer Application（消费者应用，即 “用 API 的程序”）

- 术语解释：就是我们前面举例的 “终端 APP”“设备诊断 APP”“SD 卡读取 APP”—— 是 API 的 “使用者”，通过两种方式调用 API：
  1. 直接调用：自己准备调用号 + 参数，通过胶水层的必选入口触发 syscall；
  2. 间接调用：用胶水层的可选辅助函数（比如`uart_putc`），更简单。
- 例子：应用要输出 “Hello”：
  - 直接调用：自己写`glue_syscall(0x01, 'H')`→`glue_syscall(0x01, 'e')`→…→依次输出每个字符；
  - 间接调用：如果胶水层有`console_print("Hello")`的辅助函数，直接写`console_print("Hello")`就行，函数内部会自动处理调用号和参数。

## 13. 总结

​	这个 API 本质是 U-Boot 给 “嵌入式应用” 开的一个 “服务窗口”—— 让应用能在 U-Boot 阶段（比如设备开机还没启动操作系统时），就能用 U-Boot 的硬件管理能力（读存储、联网、控制设备）。整个设计的核心是 “轻量、灵活”：U-Boot 只做基础转发，应用自己掌控逻辑，还能按需选择是否用辅助工具（胶水层包装函数），适配不同场景的需求。

## 14. 如何使用U-Boot API

要在实际应用中使用 U - Boot API，可以按照以下步骤进行：

### 14.1 确认 API 可用性

- **查找签名**：消费者应用需要在指定的地址空间范围内搜索 API 的签名，以确定 API 是否可用。比如，假设按照约定，API 签名位于内存地址 `0x100000 - 0x101000` 这个范围，应用程序需要编写代码去读取这个地址范围的数据，检查是否存在预期的签名。如果找到签名，说明 U - Boot 中已经集成了该 API 并且可以使用；如果未找到，则可能需要重新配置或编译 U - Boot 以启用该 API（如果 API 是可选的，如 CONFIG_API）。

### 14.2 准备调用上下文

- **确定调用号和参数**：根据要执行的具体功能，确定相应的调用号和参数。例如，如果要使用`putc`函数向控制台输出一个字符 ，需要明确`putc`对应的调用号（假设为`0x01`），并准备好要输出的字符参数（比如要输出字符`'A'` ，则参数就是`'A'` ）。对于更复杂的功能，如读取 SD 卡上的文件，使用`read`功能，需要准备设备标识（如 SD 卡设备名`mmc0` ）、要读取的文件名、读取数据存储的内存地址等参数。

### 14.3 进行 API 调用

- **直接调用方式**

  - **通过胶水层入口**：利用胶水层中消费者侧的必选入口函数来触发系统调用。假设胶水层提供了一个名为`uboot_syscall`的函数用于触发 syscall，那么在代码中可以这样调用：

    ```c
    // 假设putc的调用号为0x01，要输出字符'A'
    int result = uboot_syscall(0x01, 'A'); 
    if (result < 0) {
    // 处理调用失败的情况，如打印错误信息
    printf("putc API call failed\n"); 
    }
    ```

- **间接调用方式（如果有辅助包装函数）**：
  使用胶水层提供的辅助包装函数，这些函数通常对系统调用进行了封装，使代码更简洁和易读。例如，若胶水层提供了`uart_putc`函数来替代直接的`putc`系统调用，代码可以写成：

  ```c
  // 直接调用辅助函数输出字符'A'
  uart_putc('A'); 
  ```

### 14.4 处理不同功能调用

比如控制台相关操作：

- **输入输出**：如果要实现一个简单的命令行交互程序，使用`getc`获取用户输入的字符，`putc`输出提示信息或响应信息。例如：

```c
putc('>'); // 输出命令行提示符
char input = getc(); // 获取用户输入的一个字符
putc(input); // 将用户输入的字符回显
```

- **检查输入**：使用`tstc`来判断是否有用户输入，避免程序在等待输入时一直阻塞。比如：

```c
while(1) {
    if (tstc()) {
        char input = getc();
        // 处理用户输入
        break;
    }
}
```

# U-Boot驱动模型

## 1.基本概念

DM (Driver Model) 是 U-Boot 标准的 device-driver 开发模型，跟 kernel 的 device-driver 模型非常类似。

```txt
Terminology
-----------
Uclass - a group of devices which operate in the same way. A uclass provides a way of accessing individual devices within the group, but always using the same interface. For example a GPIO uclass provides operations for get/set value. An I2C uclass may have 10 I2C ports,4 with one driver, and 6 with another.

Driver - some code which talks to a peripheral and presents a higher-level interface to it.

Device - an instance of a driver, tied to a particular port or peripheral.
```

## 2.与Linux驱动的差异

U-Boot 和 Linux 内核在设备驱动初始化机制上的一个核心差异就是：**U-Boot 不会自动探测和初始化设备，必须由开发者在代码中显式调用 probe 函数来激活驱动**。

### 1.Linux 内核自动 probe

在 Linux 内核中，当系统启动时，内核会先解析设备树，识别所有硬件设备。然后遍历所有已注册的驱动，通过`compatible`属性匹配设备和驱动一旦匹配成功，内核会**自动调用驱动的 probe 函数**，完成设备初始化。整个过程是全自动的，开发者不需要手动编写代码触发 probe，内核框架会处理好一切。

### 2.U-Boot 手动 probe

在 U-Boot 中，虽然也有设备树解析和驱动注册，但**不会自动执行 probe**，必须由开发者在初始化代码（如板级初始化函数）中，通过类似以下的方式**主动调用 probe**，只有执行了这些主动调用，驱动的 probe 函数才会被执行，设备才会被初始化。

```c
// 步骤1：通过设备名获取设备并触发probe，该接口的核心调用就是device_probe
uclass_get_device_by_name(UCLASS_I2C_GENERIC, "lt8912b", &dev);

// 步骤2：直接调用probe函数
device_probe(dev);
```

## 3.probe基本原理

`device_probe()` 是 U-Boot 设备模型中最核心的函数之一，负责完成设备的初始化工作。它的主要作用是将设备（`struct udevice`）与驱动（`struct driver`）绑定，并执行一系列初始化操作，最终调用驱动的 `probe` 函数激活设备。以下是逐段代码的详细解释：

```c
/*Activate a device so that it is ready for use. All its parents are probed first.*/
int device_probe(struct udevice *dev)
{	
	//1.入参检查
    const struct driver *drv;
	int size = 0,ret,seq;
	if (!dev)//非法参数检查
		return -EINVAL;
	if (dev->flags & DM_FLAG_ACTIVATED)//设备已激活
		return 0;
	drv = dev->driver;
	assert(drv);//判断传入的设备是否绑定与之匹配的驱动，即driver是否为空
    
    //2.申请数据
	/*分配驱动私有数据（如果驱动需要且未分配）,存储驱动的私有数据（如硬件寄存器缓存、状态变量等）*/
	if (drv->priv_auto_alloc_size && !dev->priv) {
		dev->priv = alloc_priv(drv->priv_auto_alloc_size, drv->flags);
		if (!dev->priv) {
			ret = -ENOMEM;
			goto fail;
		}
	}
	/*分配设备类私有数据（如果设备类需要且未分配），用于设备类管理该设备的专属数据 */
	size = dev->uclass->uc_drv->per_device_auto_alloc_size;
	if (size && !dev->uclass_priv) {
		dev->uclass_priv = calloc(1, size);
		if (!dev->uclass_priv) {
			ret = -ENOMEM;
			goto fail;
		}
	}
    //3. 初始化父设备（依赖处理）
	/* Ensure all parents are probed */
	if (dev->parent) {
        /*分配父设备的所有子设备私有数据（如果需要）*/
		size = dev->parent->driver->per_child_auto_alloc_size;
		if (!size) {
			size = dev->parent->uclass->uc_drv->
					per_child_auto_alloc_size;
		}
		if (size && !dev->parent_priv) {
			dev->parent_priv = alloc_priv(size, drv->flags);
			if (!dev->parent_priv) {
				ret = -ENOMEM;
				goto fail;
			}
		}
		/* 递归初始化父设备（确保父设备先于子设备初始化） */
		ret = device_probe(dev->parent);
		if (ret)
			goto fail;

		/*
		 /* 父设备初始化时可能已间接初始化当前设备，需再次检查状态
		 * (e.g. PCI bridge devices). Test the flags again
		 * so that we don't mess up the device.
		 */
		if (dev->flags & DM_FLAG_ACTIVATED)
			return 0;
	}
	//4.设置设备序号与激活标志
	/*获取设备在同类设备中的序号（如 i2c0、i2c1）*/
    seq = uclass_resolve_seq(dev);
	if (seq < 0) {
		ret = seq;
		goto fail;
	}
	dev->seq = seq;// 保存序号
	dev->flags |= DM_FLAG_ACTIVATED;// 标记设备开始激活（防止中途被重复初始化）
    
	//5. 引脚配置与预处理
	/*
	 * Process pinctrl for everything except the root device, and
	 * continue regardless of the result of pinctrl. Don't process pinctrl
	 * settings for pinctrl devices since the device may not yet be
	 * probed.
	 */
    /* 为非根设备和非引脚控制器设备设置默认引脚状态 */
	if (dev->parent && device_get_uclass_id(dev) != UCLASS_PINCTRL)
		pinctrl_select_state(dev, "default"); // 应用设备树中定义的 "default" 引脚配置

	/* 设备类级别的 probe 前处理,完成类级别的初始化（如初始化同类设备的共享资源） */
    ret = uclass_pre_probe_device(dev);
	if (ret)
		goto fail;

     /* 
       父设备对当前子设备的 probe 前处理（如总线控制器准备子设备通信环境）,如果父设备的驱动定义了 child_pre_probe 方法（如         I2C 控制器为子设备设置通信参数），则调用该方法为当前设备做准备。
     */
	if (dev->parent && dev->parent->driver->child_pre_probe) {
		ret = dev->parent->driver->child_pre_probe(dev);
		if (ret)
			goto fail;
	}
    
    //6.设备树数据转换与驱动probe
	/* 
	 将设备树数据转换为驱动的平台数据（platdata）,如果驱动定义了 ofdata_to_platdata 方法（用于解析设备树属性），且设备         有设备树节点，则调用该方法将设备树中的配置（如寄存器地址、时钟频率）转换为驱动可直接使用的平台数据（dev->platdata）。 
	*/
    if (drv->ofdata_to_platdata && dev_has_of_node(dev)) {
		ret = drv->ofdata_to_platdata(dev);
		if (ret)
			goto fail;
	}
	/* 调用驱动的 probe 函数（核心初始化逻辑） */
	if (drv->probe) {
		ret = drv->probe(dev);
		if (ret) {
			dev->flags &= ~DM_FLAG_ACTIVATED;
			goto fail;
		}
	}
   //7.probe 后处理与完成
   /* 设备类级别的 probe 后处理,完成 probe 后的类级操作（如更新同类设备列表、通知其他依赖组件）。 */
    ret = uclass_post_probe_device(dev);
	if (ret)
		goto fail_uclass;
	/* 若当前设备是引脚控制器，应用初始化和默认引脚状态 */
	if (dev->parent && device_get_uclass_id(dev) == UCLASS_PINCTRL) {
		pinctrl_select_state(dev, "init");
		pinctrl_select_state(dev, "default");
	}
	return 0;
    
   //8.错误处理
   /* 若设备类后处理失败，尝试移除设备 */
 fail_uclass:
	if (device_remove(dev, DM_REMOVE_NORMAL)) {
		dm_warn("%s: Device '%s' failed to remove on error path\n",
			__func__, dev->name);
	}
   /* 清除激活标志，重置序号，释放分配的内存 */
 fail:
	dev->flags &= ~DM_FLAG_ACTIVATED;
	dev->seq = -1;
	device_free(dev);  // 释放之前分配的私有数据
	return ret;
}
```

`device_probe()` 的核心逻辑可概括为：**检查状态 → 分配资源 → 初始化父设备 → 预处理（引脚、类、父设备） → 解析设备树 → 调用驱动 probe → 后处理**。整个过程确保了设备初始化的依赖顺序（父设备优先）、资源分配（私有数据）和硬件配置（引脚、设备树参数）的正确性，最终通过驱动的 `probe` 函数完成硬件激活。

------

# FIT镜像树

## 1.概念介绍

FIT = Flattened Image Tree（扁平化镜像树），其本质就是把内核、DTB、ramdisk、签名等打包成单个 .itb 文件，用设备树语法描述结构，可以说是个是个文件格式或者容器，其内部信息由UBoot运行时解析并供后续启动使用。

传统老方式（legacy uImage）通过mkimage 只给单个内核加个 64 字节头，只能带一个 payload。要带内核+DTB+rootfs就得多步，这样很容易出错且难验证。而FIT 用一个 .its，并通过mkimage 生成 .itb镜像文件，一个文件装多组件并能自动校验。

SPL或者uboot先读取itb，再按configuration选组合，然后校验hash/签名并解压各个组件，最后把各个组件各自加载到内存运行并跳转 kernel内核。

## 2.主要用途

  - 一次烧写多文件
  - Verified Boot
    验证签名再跑，防止flash被篡改
  - 更新流程统一
    固件/内核/DTS 全部装进一个容器

## 3.核心原理

FIT 格式在不同架构芯片中实现硬件兼容性的核心逻辑是：用设备树语法描述镜像组件与硬件的匹配关系，再通过 U-Boot 运行时的硬件检测动态选择适配组件。这种机制与芯片架构（如 ARM、PowerPC、RISC-V）无关，而是通过一套通用的兼容性标识 + 匹配算法实现跨架构适配。无论芯片架构是 ARM（如瑞芯微 RK3506）、PowerPC（如 NXP MPC85xx）还是 RISC-V，FIT 都通过以下机制保证兼容性。

### 1.组件标记

FIT 镜像中的每个组件（如 U-Boot 主程序、设备树）都通过compatible属性标记它支持的硬件，格式与设备树中的`compatible` 字段一致（这是跨架构兼容的关键）。例如：

- 一个支持 RK3506 评估板的 U-Boot 组件会标记
  `compatible = "rockchip,rk3506-evb"`
- 一个支持 MPC8548 开发板的内核组件会标记
  `compatible = "fsl,mpc8548ds"`

这些标签直接对应硬件的身份标识，与架构无关。

### 2.硬件检测

U-Boot 的SPL启动时，会通过两种方式获取当前硬件的身份标识：

- **读取芯片内置 ID**
  如 ARM 芯片的 `CHIP_ID` 寄存器、PowerPC 的 `PVR`（处理器版本寄存器），确定芯片型号。
- **解析基础设备树**
  对于复杂硬件，U-Boot 会先加载一个基础设备树（通常固化在芯片或启动介质中），从中读取根节点的`compatible` 属性（如 `compatible = "rockchip,rk3506"`），作为硬件的核心标识。

例如，RK3506 开发板的基础设备树根节点会使用如下声明表示 “这是 RK3506 芯片的评估板”。

```dts
/ {
    compatible = "rockchip,rk3506-evb", "rockchip,rk3506";
};
```

### 3.匹配算法

U-Boot 会将硬件的身份标识（如 `rockchip,rk3506-evb`）与 FIT 镜像中组件的 `compatible` 标签进行比对，遵循 **最长前缀匹配”**原则：

- 优先选择与硬件标识完全一致的组件（如 `rockchip,rk3506-evb` 匹配标签相同的组件）。
- 若没有完全匹配，则匹配更通用的父级标识（如 `rockchip,rk3506` 匹配支持所有 RK3506 芯片的组件）。

这种算法与架构无关，仅依赖文本字符串比对，确保在任何架构下都能工作。

## 4.`its`文件

`.its` 文件（Image Tree Source）是描述 FIT（Flattened Image Tree）镜像结构的**源文件**，采用类似设备树（DTS）的语法，用于定义 FIT 镜像包含的组件（如内核、设备树、固件）、配置信息和兼容性规则。最终通过 `mkimage` 工具编译为`.itb` 二进制文件（FIT 镜像）。

### 1.文件的基本结构

```c
/dts-v1/;  // 版本声明

/ {
    description = "FIT 镜像描述";     // 全局描述
    #address-cells = <1>;            // 地址字段的单元格数量（类似设备树）
    #size-cells = <1>;               // 大小字段的单元格数量

    images {                         // 定义所有组件（镜像文件）
              // 组件1：如内核、U-Boot、设备树等
              image@1 {
                        // 组件属性...
              };
              // 组件2...
    };

    configurations {                  // 定义配置（组件组合方式）
        default = "conf@1";          // 默认配置
        
        // 配置1：指定使用哪些组件
        conf@1 {
            // 配置属性...
        };
        // 配置2...
    };
};
```

- `images`
  存放所有二进制组件（如内核、设备树、ramdisk），每个组件有唯一标识（如 `image@1`）。
- `configurations`
  定义组件的组合方式（如 “用内核 A + 设备树 B 启动”），U-Boot 会根据硬件兼容性选择合适的配置。

### 2.核心语法与属性

#### 1.全局属性（根节点）

- `description`
  FIT 镜像的描述文本（如 `"U-Boot FIT for RK3506"`），用户自定义，可以随便写。
- `#address-cells` 和 `#size-cells`
  指定地址和大小字段的单元格数量（通常设为 `<1>` 或 `<2>`，与硬件地址宽度匹配）。

#### 2.组件定义（images节点）

每个组件（如内核、设备树）是 `images` 的子节点，需指定以下关键属性：

|    属性名     |                             作用                             |                            示例                             |
| :-----------: | :----------------------------------------------------------: | :---------------------------------------------------------: |
| `description` |                        组件的描述文本                        |              `"Linux kernel 5.10 for RK3506"`               |
|    `type`     |                           组件类型                           | `"kernel"`（内核）、`"fdt"`（设备树）、`"firmware"`（固件） |
|    `arch`     |                           目标架构                           |               `"arm"`、`"powerpc"`、`"riscv"`               |
|     `os`      |                   操作系统（仅对内核有效）                   |                          `"linux"`                          |
| `compression` |               压缩方式（`"none"` 表示未压缩）                |                     `"gzip"`、`"none"`                      |
|    `load`     | 加载地址（组件应被加载到的内存地址，`0x0` 表示由 U-Boot 自动分配，**设备树节点不需要**） |                        `0x80080000`                         |
|    `entry`    | 入口地址（组件的执行起始地址，内核 / 固件需要，**设备树不需要**） |                        `0x80080000`                         |
|    `data`     |    组件的二进制数据（通常通过 `incbin` 指令引用外部文件）    |             `incbin("zImage")` （引用内核文件）             |
|    `hash`     |             哈希校验（可选，用于验证组件完整性）             |                `sha256` { value = "xxx"; };                 |
| `compatible`  | 组件支持的硬件（用于兼容性匹配，类似设备树的 `compatible`）  |         `"rockchip,rk3506-evb", "rockchip,rk3506"`          |

#### 3.配置定义（configurations节点）

配置节点用于组合 `images` 中的组件，告诉 U-Boot “如何使用这些组件启动”。每个配置需指定：

|    属性名     |                     作用                     |            示例值             |
| :-----------: | :------------------------------------------: | :---------------------------: |
| `description` |                配置的描述文本                | `"RK3506 EVB default config"` |
|   `kernel`    |  引用`images` 中的内核组件（如 `kernel@1`）  |         `"kernel@1"`          |
|     `fdt`     |   引用`images`中的设备树组件（如 `fdt@1`）   |           `"fdt@1"`           |
|  `firmware`   | 引用 `images` 中的固件组件（如 U-Boot 镜像） |          `"uboot@1"`          |
| `compatible`  |   该配置支持的硬件（用于 U-Boot 选择配置）   |    `"rockchip,rk3506-evb"`    |

#### 4.哈希与签名

`.its` 支持为**组件**添加哈希校验或数字签名，用于验证完整性和安全性（需 U-Boot 开启 `CONFIG_FIT_SIGNATURE`）。

```dts
images {
    kernel@1 {
        // ... 其他属性 ...
        hash@1 {
            algo = "sha256";  // 哈希算法
            value = [5e 8f ...];  // 哈希值（可通过工具生成）
        };
        signature@1 {
            algo = "rsa2048";  // 签名算法
            key-name-hint = "dev-key";  // 密钥名称
            value = [30 82 ...];  // 签名数据
        };
    };
};
```

## 5.组件定义示例

定义一个内核组件和一个设备树组件示例如下：

```dts
images {
    kernel@1 {
        description = "Linux 5.10 kernel";
        type = "kernel";
        arch = "arm";
        os = "linux";
        compression = "gzip";
        load = <0x80080000>;  // 内核加载地址
        entry = <0x80080000>; // 内核入口地址
        data = incbin("zImage.gz");  // 引用压缩后的内核文件
        compatible = "rockchip,rk3506";  // 支持所有RK3506芯片
    };

    fdt@1 {
        description = "RK3506 EVB device tree";
        type = "fdt";
        arch = "arm";
        compression = "none";
        data = incbin("rk3506-evb.dtb");  // 引用设备树文件
        compatible = "rockchip,rk3506-evb";  // 仅支持评估板
    };
};
```

## 6.默认配置示例

```dts
configurations {
    default = "conf@1";  // 默认使用 conf@1 配置
    conf@1 {
        description = "Default config for RK3506 EVB";
        kernel = "kernel@1";  // 使用前面定义的 kernel@1
        fdt = "fdt@1";        // 使用前面定义的 fdt@1
        compatible = "rockchip,rk3506-evb";  // 匹配评估板
    };
};
```

## 7.文件完整示例

```dts
/dts-v1/;
/ {
    description = "FIT image for RK3506 EVB";
    #address-cells = <1>;
    #size-cells = <1>;

    images {
        // U-Boot 固件组件
        uboot@1 {
            description = "U-Boot 2023.07 for RK3506";
            type = "firmware";
            arch = "arm";
            compression = "none";
            load = <0x00200000>;  // RK3506 的 U-Boot 加载地址
            entry = <0x00200000>;
            data = incbin("u-boot.bin");
            compatible = "rockchip,rk3506-evb";
        };

        // 设备树组件
        fdt@1 {
            description = "RK3506 EVB DTB";
            type = "fdt";
            arch = "arm";
            compression = "none";
            data = incbin("rk3506-evb.dtb");
            compatible = "rockchip,rk3506-evb";
        };
    };

    configurations {
        default = "conf@1";
        conf@1 {
            description = "RK3506 EVB boot config";
            firmware = "uboot@1";  // 使用 U-Boot 组件
            fdt = "fdt@1";         // 使用设备树组件
            compatible = "rockchip,rk3506-evb";
        };
    };
};
```

## 8.编译 .its 

通过 U-Boot 提供的 `mkimage` 工具将 `.its` 编译为 `.itb` 镜像：

```bash
mkimage -f example.its example.itb

#-f      指定.its源文件。
#输出文件 example.itb即为可被U-Boot识别的FIT镜像。
```

------

