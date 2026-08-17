# 引导程序

​	Bootloader是在操作系统运行之前运行的一小段代码，用于将软硬件环境初始化到一个合适的状态，比如初始化时钟，中断，`DRAM`控制器等外设，为操作系统的加载和运行做准备（其本身不是操作系统），再把Linux从Flash拷贝到`DRAM`中，最后再启动Linux内核。Bootloader是启动引导程序的统称，类似于操作系统与Linux的概念，嵌入式Linux常用的Bootloader是U-Boot。

------

# 基本功能

- 初始化软硬件环境，如时钟、内存、串口、存储、网络等。
- 提供调试与维护功能：如环境变量配置、固件升级、网络引导、烧写命令等。
- 引导加载Linux内核，通常是 Linux 内核，并传递启动参数、设备树等必要信息。
- 给Linux内核传参
- 执行用户命令（如U-Boot命令）

------

# U-Boot启动阶段

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

# U-Boot初始化流程

## 1.U-Boot启动过程

### 1.总体启动流程
​	在嵌入式 Linux  系统开发、BSP 适配及新平台 bring-up 过程中，Bootloader 的启动流程往往决定了整个系统移植和硬件支持的复杂度。U-Boot 作为当前主流的开源 Bootloader，其启动流程又分为带 SPL（Secondary Program Loader）与不带 SPL两种模式。不同方案在硬件要求、启动机制、代码结构、调试思路等方面存在明显区别。以 ARM Cortex-A SoC（如 NXP i.MX6/8, Allwinner, Rockchip , STM32MP1 等）为例，完整启动链路如下：

### 2.有 SPL 的启动流程

```txt
BootROM (SoC内部固件) → SPL (精简U-Boot) → U-Boot (完整版) → Linux Kernel → RootFS → User Application
```

### 3.无 SPL 的启动流程

```shell
BootROM (SoC内部固件) → U-Boot (完整版) → Linux Kernel → RootFS → User Application
```

## 2.BootROM 与 SPL 

- **BootROM**
  不可更改，由芯片原厂烧录在 SoC 内部，只负责最基础的硬件初始化和“从外部介质加载 bootloader 到 SRAM/DRAM 并运行”。
- **SPL**
  U-Boot 的精简版，用于解决“内部SRAM容量有限，无法一次加载完整版 U-Boot”时的硬件初始化需求（如 DDR、时钟、简单外设）。
- **完整版 U-Boot**
  具备命令行、丰富外设/协议栈支持，可做升级、调试、启动内核等。

## 3.有无SPL的差异

### 1.SPL 设计的核心动机

#### 1.SPL 解决的核心问题

​	SOC 上电后，可用的 SRAM 极其有限，无法直接放下大体积的 U-Boot。因此，必须先用极小的 SPL 初始化 DDR，之后将完整版 U-Boot 从外部介质加载到 DDR，再切换 到 U-Boot 运行。

#### 2.场景举例

​	典型 Cortex-A9/A53/A72 SoC，SRAM 只有 32KB/64KB/128KB，而完整版 U-Boot 动辄数百 KB 甚至上 MB。同时需要复杂 的DRAM 初始化脚本，BootROM 能力有限。

### 2.无 SPL 方案的条件

​	SOC 内部 SRAM 或 BootROM 能直接完成 DDR 初始化。BootROM 能直接从存储加载大容量 U-Boot 到 DDR 并运行。U-Boot 镜像尺寸可以接受（如 NOR flash 启动、小型芯片/简单板卡）。

#### 1.场景举例

早期的 ARM9、ARM11、部分 Cortex-M4/M7，以及 MCU 级应用，内存需求不高。使用 NOR Flash，直接映射到地址空间，无需分段加载。BootROM 足够灵活/支持的 SoC（如部分 Allwinner、Rockchip、早期NXP平台等）。

## 4.详细启动流程对比

### 1.有 SPL 启动流程详解

1. **BootROM 执行**
   SOC 上电后，BootROM 按固定顺序检测外部存储（SD/eMMC/NAND/NOR/USB等），找到并加载 SPL 到 SRAM。
2. **SPL 阶段**
   初始化必要硬件（主要是时钟/DDR）。极简驱动，仅包含板级初始化和加载U-Boot代码的功能。加载完整 U-Boot 镜像到 DDR。跳转至 U-Boot：SPL 跳转到 DDR 内的 U-Boot 入口，后者接管控制权。

3. **U-Boot 主阶段**
   初始化更丰富的外设。提供 CLI/网络/升级/调试等功能。加载和启动 Linux 内核。操作系统启动。


#### 1.核心代码片段

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

#### 2.SPL 配置

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

#### 1.核心代码片段

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

#### 2.配置特征

.config 文件里无 CONFIG_SPL 相关选项。编译输出只有单个 U-Boot 镜像，无 u-boot-spl 文件。

## 5.适用场景分析

### 1.带 SPL 的优势和适用场景

​	适合中高端 SOC，SRAM 远小于 U-Boot 镜像体积。支持复杂的板级初始化需求（如多片 DDR、PMIC、丰富外设）。支持多存储介质和更灵活的启动策略。可拆分初始化流程，有利于分阶段调试和维护。

### 2. 无 SPL 的优势和适用场景

​	平台简单、硬件初始化需求低，BootROM 或 NOR Flash 直映射即可。启动速度快，代码复杂度低，适用于量产型、成本敏感的设计。有利于初学者和小型项目快速上手。

## 6.开发实践总结

### 1.如何判断需要 SPL

​	检查 SoC 文档“BootROM 支持的最大加载大小”、“上电后可用的 SRAM/DRAM 初始化流程”。**若 U-Boot 体积超过 BootROM 一次性可加载容量**，必须引入 SPL。

​	若平台/芯片厂商提供的参考 U-Boot 都有 SPL 分段，说明其硬件需要这种启动分阶段。

### 2.编译与适配流程

- **有 SPL**
  	编译时会生成 u-boot-spl/u-boot-spl.bin，同时 u-boot/u-boot.bin/elf，烧录脚本须注意 SPL 与主镜像分区、拼接位置等。
- **无 SPL**
          只有主镜像直接烧录，启动配置/烧写脚本更简单。

### 3.调试和移植

- 有 SPL 时遇到 DDR 问题，建议优先只修改/调试 SPL，主 U-Boot 尽量保持稳定。
- 无 SPL 时，所有初始化和调试均集中于 U-Boot 代码，调试窗口更大但也更易出错。

### 4.示例场景

```c
Q1：我怎么判断我的平台当前用不用 SPL？
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

## 7.无SPL的一个疑问

### 1.启动执行过程
```bash
#BootROM 执行： 
	SOC 上电后，BootROM 直接从外部存储加载完整 U-Boot 镜像到内存（一般是 DDR）并执行。 
#U-Boot 阶段： 
	U-Boot 自行初始化全部硬件（包含 DDR）。 后续流程同上：提供 CLI、升级、内核引导等功能。 
#问题引出：
	U-Boot镜像直接加载到DDR运行，那为啥U-boot后面还要初始化DDR？
```

### 2.解释
​	这个问题的核心在于：**BootROM 对 DDR 的初始化可能是 “临时且有限的”，而 U-Boot 对 DDR 的初始化是 “完整且适配系统需求的”**。两者的目标和能力存在本质差异，具体原因如下：

#### 1.BootROM 的 DDR 操作

​	仅为 “加载镜像” 服务，而非 “完整使用”。BootROM 是 SOC 出厂时固化在芯片内部的极简程序，它的核心目标是 **“把 U-Boot 镜像从外部存储（如 eMMC、SPI Flash）加载到内存（DDR）并启动”**，而非 “为整个系统提供稳定可用的 DDR 环境”。其对 DDR 的操作通常有以下限制：

1. **初始化程度有限**
   BootROM 可能仅完成 DDR 的 **“最小化初始化”**（如基本时序配置、电压设置），足以让 DDR 临时存储数据，但未优化性能（如未设置最高频率、未启用多通道、未校准信号完整性）。
   例如：某 SOC 的 DDR 支持 1600Mbps，但 BootROM 可能仅以 400Mbps 初始化，仅保证 “能加载镜像”，无法满足后续系统对带宽的需求。
2. **依赖固定配置**
   BootROM 的 DDR 配置是 **“通用预设”**（针对该 SoC 支持的主流 DDR 型号），无法适配所有定制化硬件。
   例如：若开发板使用了 BootROM 预设之外的 DDR 型号（如不同容量、不同厂商的颗粒），BootROM 的初始化可能失败或不稳定，导致 U-Boot 镜像加载后无法正常执行。
3. **不负责内存管理**
   BootROM 仅关心 “将 U-Boot 镜像加载到 DDR 的某个地址”，但不会划分内存区域（如预留内核空间、设备树空间、环境变量空间），也不会检测内存错误（如坏块）。

#### 2.U-Boot 初始化 DDR 

​	核心目的是为 “全系统” 提供可靠内存环境，U-Boot 作为系统启动的 “总管家”，需要确保 DDR 满足后续所有操作（自身运行、内核加载、用户交互等）的需求，因此必须重新初始化 DDR，具体包括：

1. **精确适配硬件**
   U-Boot 会根据 **板级配置**（如 `board/xxx/xxx.c` 中的 DDR 参数）和 DDR 芯片手册，配置最匹配的时序（tCL、tRCD 等）、频率、电压、通道模式（单通道 / 双通道），确保 DDR 在当前硬件上稳定运行。
   例如：开发板使用了 3200Mbps 的 LPDDR4，U-Boot 会将其配置到最高频率，而 BootROM 可能仅用 1600Mbps 加载镜像。
2. **完善内存管理**
   U-Boot 会划分 DDR 内存布局（通过 `global_data` 和内存池管理），明确哪些区域用于 U-Boot 自身、哪些用于内核、哪些用于设备树、哪些用于环境变量等，避免内存冲突。同时，U-Boot 可能会执行内存检测（如 `memtest` 命令），标记坏块，确保后续使用的内存区域可靠。
3. **支持高级功能**
   部分场景下，U-Boot 需要启用 DDR 的高级特性（如 ECC 错误校验、内存加密、低功耗模式），这些功能通常超出 BootROM 的能力范围，必须由 U-Boot 初始化。
4. **兼容 “无 BootROM 初始化” 的场景**
   并非所有 SOC 的 BootROM 都会初始化 DDR。
   例如：部分低成本芯片的 BootROM 仅能从 SPI Flash 加载 U-Boot 到 SRAM（而非 DDR），此时 U-Boot 必须完全从零开始初始化 DDR，否则无法使用大容量内存。为了统一代码路径，即使 BootROM 做了部分初始化，U-Boot 也会执行完整流程，确保兼容性。

### 3.典型流程

BootROM 与 U-Boot 的 DDR 操作分工，以常见的 ARM 架构 SOC 为例，流程通常是：

1. **BootROM 阶段**
   - 上电后执行内部 ROM 程序，尝试从外部存储（如 eMMC）读取 U-Boot 镜像。
   - 若需要加载到 DDR，会用预设的极简参数临时初始化 DDR（仅保证能存数据），然后将 U-Boot 镜像加载到 DDR 的指定地址（如 `0x80080000`）。
   - 跳转到 DDR 中的 U-Boot 入口（此时 U-Boot 开始执行，但 DDR 尚未完全就绪）。
2. **U-Boot 阶段**
   - 进入 `lowlevel_init` 或 `board_init_f` 函数，首先执行 **完整的 DDR 初始化**（覆盖 BootROM 的临时配置），设置正确的时序、频率等参数。
   - 初始化完成后，U-Boot 会将自身**重定位**到 DDR 的合适位置（如内存顶部），并划分内存区域。
   - 后续流程（加载内核、启动系统）均基于 U-Boot 初始化后的稳定 DDR 环境。

### 4.总结

​	BootROM 对 DDR 的操作是 “临时应急”，仅为加载 U-Boot 服务；而 U-Boot 对 DDR 的初始化是 “系统级准备”，为整个启动流程（包括自身运行、内核加载）提供稳定、高性能、适配硬件的内存环境。两者目标不同，因此 U-Boot 必须重新初始化 DDR，不能依赖 BootROM 的有限操作。

# 板级初始化流程

> [!NOTE]
>
> 参考U-Boot源码根目录下的**README**。

## 1.整体流程概述

​	此流程板卡（boards）预设的启动流程（intended start-up flow”，即硬件板卡正常启动时应遵循的标准步骤。该启动流程对两类程序均适用 ——**SPL（Secondary Program Loader，二级程序加载器）** 和 **U-Boot proper（标准 U-Boot 程序）**，且二者需遵循相同规则（follow the same rules）。

```c
//U-Boot（包括 SPL 和主 U-Boot）的板级初始化遵循统一规则，核心流程为：
架构相关的start.S → lowlevel_init() → board_init_f() → （BSS 清除 + U-Boot镜像重定位） → board_init_r() → main_loop()

    
//main_loop：uboot-主循环
//起点：从架构 / CPU 专属的start.S开始（如 ARMv7 的 `arch/arm/cpu/armv7/start.S`），这是汇编级别的启动入口。
//核心目标：逐步搭建硬件环境（从最基础的执行条件到完整的内存 / 外设可用），最终进入 U-Boot 主循环处理命令和启动系统。
```

## 2.BSS 清除与重定位

在 `board_init_f()` 完成后、`board_init_r()` 执行前，会**自动**进行两项关键操作：

- **BSS 清除**：清空未初始化的全局变量区域（确保全局变量初始值为 0）。
- 重定位（根据配置）：
  - **SPL**：若定义 `CONFIG_SPL_STACK_R`，栈和 `global_data` 会被迁移到 `CONFIG_SPL_STACK_R_ADDR` 下方。
  - **主 U-Boot**：整个 U-Boot 镜像被重定位到内存顶部。

## 3.三大核心函数

### 1.`lowlevel_init()`

1. **核心作用**
   是启动过程中最早期的初始化函数，仅完成最基础的设置，确保程序能正常执行到`board_init_f()`函数。
2. **限制条件**：
   - 不依赖`global_data`（全局数据结构）和 BSS 段（未初始化数据区）
   - 基本没有栈（ARMv7 架构可能有临时栈，但很快会被移除）
   - 禁止初始化 SDRAM（内存）和使用控制台输出
   - 只做让程序能继续执行到`board_init_f()`的最低限度工作
3. **使用场景**：几乎不需要自定义实现这个函数（**通常使用默认实现**）
4. **执行要求**：需要正常返回，不能在此处终止执行流程。

### 2.`board_init_f()`

1. **核心作用**：为`board_init_r()`函数的运行做准备，重点初始化 SDRAM（同步动态随机存储器）和串行 UART（通用异步收发传输器），使硬件达到可运行后续初始化程序的状态。
2. **运行环境**：
   - 可以访问`global_data`（全局数据结构**GD**，通常通过一个宏定义如 `DECLARE_GLOBAL_DATA_PTR`访问，本质上是一个指向 SRAM 中特定位置的指针，而**不属于传统意义上的全局变量**）
   - 栈（stack）位于 SRAM（静态随机存储器）中
   - BSS 段（未初始化的全局变量区域）不可用，因此不能使用全局变量或静态变量，只能使用栈变量和`global_data`
3. **非 SPL 相关说明**：
   - 会调用`dram_init()`来初始化 DRAM（动态随机存储器）
   - 若在 SPL（Secondary Program Loader，次级程序加载器）中已完成 DRAM 初始化，此函数可什么都不做
4. **SPL 相关说明**：
   - 可根据需要用自定义版本覆盖整个`board_init_f()`函数
   - 在极端情况下可在此处调用`preloader_console_init()`
   - 需完成 SDRAM 初始化以及 UART 工作所需的所有设置
   - 无需清理 BSS 段，这部分工作会由`crt0.S`完成
   - 必须正常返回，不能直接调用`board_init_r()`

### 3.`board_init_r()`

- **主要作用**：承担 U-Boot 启动阶段的主要执行逻辑，包含通用初始化代码，最终会跳转到 `main_loop()` 进入命令行交互阶段。
- **运行环境**：
  - 可访问 `global_data` 全局数据结构
  - SDRAM 已初始化可用
  - BSS 段已准备好，所有静态 / 全局变量可正常使用
- **非 SPL 场景特性**：
  U-Boot 已重定位到SDRAM内存顶部，并从该位置运行。
- **SPL（Secondary Program Loader）场景特性**：
  - 若定义了 `CONFIG_SPL_STACK_R` 且 `CONFIG_SPL_STACK_R_ADDR` 指向 SDRAM，栈可位于 SDRAM 中
  - 可在此处调用 `preloader_console_init()`（通常通过开启 `CONFIG_SPL_BOARD_INIT` 并实现 `spl_board_init()` 函数来完成）
  - 负责加载 U-Boot 主程序，或在 falcon 模式下直接加载 Linux 内核

### 4.SPL 与主 U-Boot 的共性与差异

|      维度      |                  SPL（第二阶段程序加载器）                   |          主 U-Boot（U-Boot proper）          |
| :------------: | :----------------------------------------------------------: | :------------------------------------------: |
| **初始化流程** | 遵循相同的三阶段函数（`lowlevel_init` → `board_init_f` → `board_init_r`） |                     同上                     |
|  **代码路径**  |              通常有独立代码路径（`spl/` 目录）               |     主代码路径（`common/`、`board/` 等）     |
|  **核心目标**  |       快速初始化 SDRAM 和串口，加载主 U-Boot 或 Linux        | 完成完整初始化，提供命令行交互，启动操作系统 |
|  **资源限制**  |               体积小，依赖 SRAM 运行，功能精简               |  功能丰富，依赖 SDRAM 运行，可使用全局变量   |
|   **重定位**   |              栈和 `global_data` 可迁移到 SDRAM               |           整个镜像重定位到内存顶部           |

## 4.整体流程

首先对开发板上的软硬件环境做进一步初始化，然后将Linux内核、设备树(dtb)、根文件系统(rootfs)从外部存储器（或网络）搬移到内存，然后跳转到linux运行。Linux开始运行后先对系统环境做初始化，当系统启动完成后，Linux再从内存中（或网络）挂载根文件系统。

大致过程如下：

1. 第一阶段：初始化时钟，关闭看门狗，关中断，关闭MMU，TLB，DCACHE，ICACHE等，初始化SDRAM控制器，初始化NAND FLASH控制器等。

2. 第二阶段：初始化一个串口用于调试打印，检测系统内存映射，将内核映像和根文件系统映像从Flash上读到DRAM空间中，为内核**设置启动参数**，调用内核。

这种分层设计既保证了对不同硬件的兼容性（通过板级函数定制），又维持了 U-Boot 核心逻辑的统一性（通用代码路径），同时 SPL 与主 U-Boot 共享同一套规则，简化了跨阶段开发。
