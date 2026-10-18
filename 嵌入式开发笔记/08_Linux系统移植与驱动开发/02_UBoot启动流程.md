# 引导程序概念

Bootloader是在操作系统运行之前运行的一小段代码，用于将软硬件环境初始化到一个合适的状态，比如初始化时钟，中断，`DRAM`控制器等外设，为操作系统的加载和运行做准备（其本身不是操作系统），再把Linux从Flash拷贝到`DRAM`中，最后再启动Linux内核。Bootloader是启动引导程序的统称，类似于操作系统与Linux的概念，嵌入式Linux常用的Bootloader是U-Boot。

------

# 引导程序功能

- 初始化软硬件环境，如时钟、内存（SRAM）、串口、存储、网络等。
- 提供调试与维护功能
  如环境变量配置、固件升级、网络引导、烧写命令等。
- 引导加载Linux内核，通常是 Linux 内核，并传递启动参数、设备树等必要信息。
- 给Linux内核传参
- 执行用户命令（如U-Boot命令）

# U-Boot安全启动

## 1.安全引导

安全引导（Secure Boot）功能是指在系统的整个启动过程中，使用**链式**验证电子签名的方式来验证系统中重要镜像文件的可靠性，然后再加载镜像文件的引导过程。安全引导功能可以保护二级厂商系统的独立性和完整性。安全引导功能的原理就是采用链式验签的方式启动系统，也就是在系统启动过程中，在加载下一个阶段的镜像之前都会对需要被加载的镜像文件进行电子验签，只有验签操作通过后，该镜像才能被加载到内存中，然后系统才会跳转到下一个阶段继续执行，整个验签链中的任何一环验签失败都会导致系统挂死，系统启动过程中的第一级验签操作是由Chip ROM来完成的。

只要芯片一出厂，用户就无法修改固化在芯片中的这部分代码，因此无法通过修改第一级验签结果来关闭安全引导功能。而且验签操作使用的RSA公钥或者哈希值将会被保存在OTP（**一次性可编程存储器**）/efuse（**电子熔丝**）中，该区域中的数据一般只有Chip ROM和TEE能够读取且无法被修改。RSA公钥或者哈希值将会在产品出厂之前被写入到OTP/efuse中，而且不同厂商使用的密钥会不一样。

![image-20260907164537123](D:\_Note\嵌入式开发笔记\figure\image-20260907164537123.png)

## 2.ARM Trust Zone简介

Trust Zone 是 ARM 为设备安全开发的一个硬件架构。它通过**硬件隔离的方式**将系统分为了两个环境——Non Secure 和 Secure ，也叫做 Normal 和 Trusted。硬件隔离是说是使用 TrustZone 可以将设备中的外设（UART、I2C、SPI、DDR 等）分为两个部分，其中一部分可以在 Normal 环境下进行操作，另外一部分在 Trusted 的环境下进行操作。硬件隔离实现在两个方面：芯片外设隔离、内存隔离（对于DDR，需要将整个区域划分为若干安全和非安全区域。通过TZASC可实现）。

## 3.ATF简介

TF(Trusted Firmware)是ARM在Armv8引入的**安全解决方案**，它的核心功能为对启动和运行过程进行特权级划分，为安全提供了整体解决方案。相较于Armv7中的Trust Zone（TZ）进行了提高，它包括启动和运行过程中的特权级划分，**实现了启动过程信任链的传导，细化了运行过程的特权级区间**。TF实际有两种Profile，对ARM Profile A的CPU应用TF-A，对ARM Profile M的CPU应用TF-M。我们一般接触的都是TF-A，又因为这个概念是ARM提出的，有时候也缩写做ATF（ARM Trusted Firmware）。

EL = **Exception Level**（异常级别）。CPU 上电启动或遇异常/中断时，会在不同特权层切换，这些层就叫异常级别。ARM64 分 4 层，数字越大权限越高：

```txt
EL0  应用程序层   ← Linux 用户程序跑这
EL1  内核层       ← Linux 内核跑这
EL2  Hypervisor   ← 虚拟机管理层，没虚拟化就跳过
EL3  最高层       ← 安全监控，ATF/BL31 跑这，管 Secure/Normal 切换
```

启动顺序：**EL3（上电）→ 往下掉到 EL1 → EL0**。每层能管下面，下面碰不到上面。再加两个维度：

- Secure World 里 EL1 = **S-EL1**，跑 TEE OS（OP-TEE）。
- Normal World 里 EL1 = **NS-EL1**，跑 Linux。

![img](..\figure\2207466-20250119154916325-237413260.png)

## 4.ATF 固件代号

ARM Trusted Firmware（ATF）把启动分成几个阶段，按 **BL（Boot Loader）+ 数字**命名

![img](..\figure\2207466-20250119160956027-82102496.png)

```txt
BL1  第一阶段引导    最底层，上电先跑
BL2  第二阶段引导    加载后面的镜像
BL31  EL3 运行时固件 Secure Monitor，常驻 EL3，管世界切换bl31包含两部分功能，
	 在启动时作为启动流程的一部分，执行软硬件初始化以及启动bl32和bl33镜像。
     在系统启动完成后，将继续驻留于系统中作为runtime，并处理来自其它异常等级的smc异常，以及其它需要路由到EL3处理的中断等。
BL32  Secure World 固件 = TEE OS（OP-TEE）
BL33  Normal World 固件 = U-Boot / EDK2

BL31、BL32和BL33都是由BL2加载的，其中BL31和BL32是可选的，若系统不支持TRUST OS，则可去掉BL32，若不支持EL3异常等级及secure monitor，则可去掉BL31
```

| ATF 代号 | 对应 Rockchip                       |
| -------- | ----------------------------------- |
| BL1      | BootROM（芯片内置）                 |
| BL2      | TPL/SPL（idbloader.img）            |
| BL31     | ATF，在 trust.img 里                |
| BL32     | OP-TEE（TEE OS），也在 trust.img 里 |
| BL33     | U-Boot                              |

## 5.ARM不同架构安全

### 1.ARMv7

ARMv7 只有 EL 概念没有 EL3（沿用 old-style 特权模式），没有 ATF 标准。可信执行环境就是 TOS（Trusted OS
  = OP-TEE） 本身。所以在RockChip的uboot编译脚本里可以看到：

```shell
  if [ "${ARM64_TRUSTZONE}" == "y" ]; then
      ${SCRIPT_ATF} --ini ${INI_TRUST} ...   # ARM64：ATF，产出 bl31 → trust.img，bl31 = Boot Loader 阶段 3 的第 1 个组件，正好是常驻 EL3 的安全监控固件
  else
      ${SCRIPT_TOS} --ini ${INI_TRUST} ...   # ARM32：直接打 TOS → trust.img，ARM32 没 EL3/ATF，安全固件直接是 TOS。
  fi
```

### 2.ARMv8

 ARMv8-A 是架构名，它定义两种执行状态：

  - AArch64 —— 64 位
  - AArch32 —— 32 位

  同一个 ARMv8 核，可以跑 AArch64，也可以跑 AArch32（向后兼容 ARMv7 指令）。所以 ARM32 完全可以是 ARMv8——只是它跑在
  AArch32 状态。

|     情况     |     核     | trust镜像 |
| :----------: | :--------: | :-------: |
|   纯 ARMv7   | 只有 32 位 |    TOS    |
| ARMv8 + 64位 |  AArch64   |    ATF    |
| ARMv8 + 32位 |  AArch32   |    ATF    |

 关键：ARMv8 核有 EL3 / TrustZone，AArch64、AArch32 状态都有。ARMv7 核没有 EL3。

### 3.总结对比

首次两种架构的安全启动都遵循下列流程：

```bash
BootROM → loader(带DDR init) → trust.img → uboot → kernel

#loader 把 trust.img 搬进安全内存，再跳 U-Boot。U-Boot 要干安全相关的活（开副核、复位、进 TEE）就发 SMC指令陷进安全世界，由 trust 那块处理。
```

 两者差别在「安全世界是谁」：

  ARMv8：trust.img = 安全监控器本身（EL3）

  - 内容 = bl31（ATF 的 EL3 Runtime），可选再拼 bl32（OP-TEE）
  - 跑在 EL3，就是常驻的 secure monitor
  - 职责：世界切换（secure↔non-secure，靠 SMC）、PSCI（CPU 开/关/复位/电源）都在这里、EL3 异常处理
  - OP-TEE 是独立的 bl32，跑 Secure-EL1，负责可信 OS 服务（TEE 调用）。打不打进同一个 trust.img 看配置

```bash
  EL3   bl31   #← trust.img 主体，常驻
  EL1S  bl32(OP-TEE)  #← 可选，也常进 trust.img
  EL1NS bl33   #= U-Boot
```

  ARMv7：trust.img = 安全 OS（TOS / OP-TEE）

  - 内容 = TOS（Trusted OS，即 OP-TEE），没有 bl31
  - ARMv7 没 EL3 概念。安全世界用 TrustZone 的 monitor mode 管切换，OP-TEE 自己兼任
  - 职责：跑到 secure 侧当可信 OS、处理 TEE 调用，PSCI 也由它提供（或内嵌 PSCI stub）

```bash
  Mon   OP-TEE(TOS)  #← trust.img，兼做 monitor
  EL1NS  #U-Boot
```

## 6.参考链接

[ATF引导启动流程整理-Part1：简介部分 - silas12138 - 博客园](https://www.cnblogs.com/silas041301/p/18679630)
[ATF引导启动流程整理-Part2：BL1引导启动流程整理 - silas12138 - 博客园](https://www.cnblogs.com/silas041301/p/18679660)
[ATF引导启动流程整理-Part3：BL2引导启动流程整理 - silas12138 - 博客园](https://www.cnblogs.com/silas041301/p/18679684)
[安全启动 - 一只Tom的笔记本](https://tomsjtu.github.io/notes/arm/secure_boot/#bl1)
[ATF入门-4BL31启动流程分析 - 知乎](https://zhuanlan.zhihu.com/p/2025983597237028579)

# U-Boot初始化阶段（老版本）

首先对开发板上的软硬件环境做进一步初始化，然后将Linux内核、设备树(dtb)、根文件系统(rootfs)从外部存储器（或网络）搬移到内存，然后跳转到Linux运行。Linux开始运行后先对系统环境做初始化，当系统启动完成后，Linux再从内存中（或网络）挂载根文件系统。

## 1.第一阶段

初始化时钟，关闭看门狗，关中断，关闭MMU，TLB，DCACHE，ICACHE等，并初始化SDRAM，初始化NAND FLASH等。

**关闭 cache 前为什么要先失效：**

涉及的两个维护操作，语义不同，不可互换：

| 操作              | 语义                       | 结果                             |
| ----------------- | -------------------------- | -------------------------------- |
| clean             | 将 dirty cache line 写回内存 | 数据同步至 DRAM                  |
| invalidate        | 将 cache line 标记为无效     | 内容作废，后续访问重新从内存读取 |
| clean & invalidate | 两者合并                   | 先写回，再作废                  |

- 关闭 cache：置 `SCTLR.C = 0`，CPU 后续访问不再经过 cache。cache 的物理存储（SRAM）内容保持不变。
- 失效 cache：将 cache line 标记为无效，内容作废。

```txt
首先，我们为何要关闭mmu？
mmu负责从虚拟地址到物理地址之间的转换，但是我们现在的汇编都是直接操作物理寄存器， 此时如果打开了mmu，而我们并没有有效的TLB，这样cpu可以说是胡乱运行的，所以我们需要关闭mmu，不需要它转换地址，直接操作寄存器方便快捷。 

然后，再发出灵魂拷问，为何要关闭cache？
因为cache和MMU是通过cp15管理的，刚上电的时候，CPU并不能管理他们。所以上电的时候mmu必须关闭，指令cache可关闭，可不关闭，但数据cache一定要关闭，否则可能导致刚开始的代码里面，去取数据的时候，从cache里面取，而这时候RAM中数据还没有cache过来，导致数据预取异常。
```

## 2.第二阶段

初始化一个串口，检测系统内存映射，将内核映像和根文件系统映像从Flash上读到DRAM空间中，为内核**设置启动参数**，调用内核。这种分层设计既保证了对不同硬件的兼容性（通过板级函数定制），又维持了 U-Boot 核心逻辑的统一性（通用代码路径），对于带SPL启动流程的， SPL 与主 U-Boot 也共享同一套规则，这种设计简化了跨阶段开发。

## 3.参考链接

[u-boot剖析（一）----Makefile分析 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4206975.html)
[u-boot分析（二）----工作流程分析 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4215627.html)
[u-boot分析（三）---boot命令实现以及内核的启动 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4217171.html)
[u-boot分析（四）---设置异常向量表|设置SVC模式 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4219919.html)
[u-boot分析（五）----I/D cache失效|关闭MMU和cache|关闭看门狗 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4222669.html)
[u-boot分析（六）----时钟初始化 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4227435.html)
[u-boot分析（七）----内存初始化 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4234451.html)
[u-boot分析（八）----串口初始化 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4237498.html)
[u-boot分析（九）----nand flash初始化|nand flash读写分析 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4240326.html)
[u-boot分析（十）----堆栈设置|代码拷贝|完成BL1阶段 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4242731.html)
[u-boot分析（十一）----MMU简单分析|u-boot分析大结局|学习规划 - wrjvszq - 博客园](https://www.cnblogs.com/wrjvszq/p/4246634.html)

# U-Boot多阶段启动（新版本）

在嵌入式 Linux  系统开发、BSP 适配及新平台 bring-up 过程中，Bootloader 的启动流程往往决定了整个系统移植和硬件支持的复杂度。U-Boot 作为当前主流的开源 Bootloader，其启动流程又分为带 SPL（Secondary Program Loader）与不带 SPL两种模式。不同方案在硬件要求、启动机制、代码结构、调试思路等方面存在明显区别。以 ARM Cortex-A SoC（如 NXP i.MX6/8, Allwinner, Rockchip , STM32MP1 等）为例，完整启动链路如下

## 1.一些概念

- **BootROM**
  不可更改，由芯片原厂烧录在 SoC 内部，只负责最基础的硬件初始化和从外部介质加载 bootloader 到 SRAM/DRAM 并运行。
- **SPL**
  U-Boot 的精简版，用于解决内部SRAM容量有限，无法一次加载完整版 U-Boot时的硬件初始化需求（如 DDR、时钟、简单外设）。
- **完整版 U-Boot**
  具备命令行、丰富外设/协议栈支持，可做升级、调试、启动内核等。

## 2.有 SPL 

```txt
BootROM (SoC内部固件) → SPL (精简U-Boot) → U-Boot (完整版) → Linux Kernel → RootFS → User Application

SOC上电后，可用的SRAM极其有限，无法直接放下大体积的U-Boot。因此，必须先用极小的SPL初始化DDR，之后将完整版U-Boot从外部介质加载到DDR，再切换到U-Boot运行。因为典型Cortex-A9/A53/A72 SoC，SRAM只有32KB/64KB/128KB，而完整版U-Boot动辄数百 KB 甚至上 MB。同时需要复杂的DRAM初始化脚本，BootROM	能力有限。
```

## 3.无 SPL 

```shell
BootROM (SoC内部固件) → U-Boot (完整版) → Linux Kernel → RootFS → User Application

SOC内部SRAM或BootROM能直接完成DDR初始化。BootROM能直接从存储加载大容量U-Boot到DDR并运行。U-Boot镜像尺寸可以接受（如NOR flash 启动、小型芯片/简单板卡）。早期的ARM9、ARM11、部分Cortex-M4/M7，以及MCU级应用，内存需求不高。使用NOR Flash，直接映射到地址空间，无需分段加载。BootROM足够灵活/支持的SoC（如部分 Allwinner、Rockchip、早期NXP平台等）。
```

## 4.补充

不能将无 SPL归因于SRAM 或 BootROM 能直接完成 DDR 初始化。因为**SPL 的字面含义是第二个程序（Secondary Program Loader）**，描述的是启动链中的位置（紧随 BootROM 之后的第二级），而非硬件容量。按 ATF 术语，SPL 对应 **BL2**：

> BL2 can be implemented by ATF, or replaced by SPL.

即同一位置可由不同来源填充。因此，**编译产物中有无 SPL，只说明 U-Boot 是否承担第二级加载器的角色，不能反推 SRAM 的容量。**

DDR 初始化必须有人执行，承担者随平台而异：

| 承担者                     | 例                              |
| -------------------------- | ------------------------------- |
| BootROM 自身               | 极简单 SoC                      |
| 厂商 BL1 / BL2 私有固件    | **Exynos4412**                  |
| U-Boot SPL（源码树内）     | 大多数 Cortex-A 平台            |
| ATF BL2                    | STM32MP、Rockchip 等            |
| pre-BL1 独立 first stage   | Socionext UniPhier              |
| DDR training firmware blob | 现代 Rockchip / i.MX8           |

**"无 SPL"应区分为三种情况：**

| 情况             | 含义                                                | 例                          |
| ---------------- | --------------------------------------------------- | --------------------------- |
| A. 厂商固件代劳  | BL1 / BL2 已完成 DDR 初始化，U-Boot 占 BL33 位置    | Exynos4412                  |
| B. BootROM 直载  | BootROM 可直接加载完整镜像，或 NOR 直接映射         | 早期 ARM9、NOR 启动、简单板卡 |
| C. 配置未开启    | 平台支持 SPL，但 `CONFIG_SPL` 未使能                | 配置问题，非硬件问题        |

**只有 B 与"SRAM 容量足够"相关。A 属分工问题，C 属配置问题。三者现象相同，成因不同。**

#### 实证：Exynos4412

U-Boot 官方 `doc/README.odroid` 明确说明该版本 U-Boot **不实现 SPL**，因此 BL1、BL2、TrustZone 三个二进制全部必需，缺一不可启动。启动链：

```
iROM (BL0) → BL1 → BL2 + TrustZone → U-BOOT
```

| 二进制      | 来源              | 职责                                  |
| ----------- | ----------------- | ------------------------------------- |
| `bl1.bin`   | 三星，签名且加密  | 芯片相关的早期初始化；非自由、不可再分发 |
| `bl2.bin`   | 板厂（Hardkernel）| **初始化 SDRAM**，加载 U-Boot          |
| `tzsw.bin`  | 厂商              | TrustZone 软件                        |

烧录偏移（SD，block = 512 B）：BL1 @1、BL2 @31、U-Boot @63、Tzsw @2111。

即：该平台的 DDR 初始化由三星私有固件完成，U-Boot 编译时自然不需要 SPL。**这与 SRAM 容量无关。** U-Boot 在此处占用的是 **BL33（non-trusted firmware）** 位置。

#### 反证：Exynos850（WinLink E850-96）

同族 SoC 的另一块板，U-Boot 文档给出的理由完全相同：

> since BL2 bootloader already sets up DRAM and runs the final bootloader (U-Boot) from DRAM, there is no need in U-Boot SPL.

依据是"BL2 已完成 DRAM 初始化"，而非 SRAM 容量。此外，Exynos 平台上也可改用 U-Boot 自身的 SPL 充当 BL2 位（见 `arch/arm/mach-exynos/spl_boot.c`），此时仅 BL1 必须来自厂商。**同一 SoC 家族两条路线皆可行，进一步说明有无 SPL 属方案选择，而非硬件判据。**

### 2.判断是否需要 SPL 的依据

1. **BootROM 一次性可加载的最大尺寸**（查 SoC 手册）。完整 U-Boot 超过此值即必须增加中间一级。
2. **上电可用 SRAM 容量**。需容纳完整 U-Boot 及其栈、`global_data`、BSS。
3. **是否已有其他固件承担早期初始化**（厂商 BL1 / BL2 或 ATF BL2）。· 

注意第 2 条位于链条下游：SPL 自身过大时还需再增加 **TPL**，形成 BootROM → TPL → SPL → U-Boot。每一级的存在理由都是"上一级装不下下一级"。

## 4.启动对比

### 1.有 SPL 

1. **BootROM 执行**
   SOC上电后，BootROM按固定顺序检测外部存储（SD/eMMC/NAND/NOR/USB等），找到并加载SPL到SRAM。
2. **SPL 阶段**
   初始化必要硬件（主要是时钟/DDR）。极简驱动，仅包含板级初始化和加载U-Boot代码的功能。加载完整U-Boot镜像到 DDR。跳转至 U-Boot，SPL 跳转到 DDR 内的 U-Boot 入口，后者接管控制权。

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
   
3. **U-Boot 主阶段**
   初始化更丰富的外设。提供 CLI/网络/升级/调试等功能。加载和启动 Linux 内核。操作系统启动。
   
   ```Kconfig
   #（典型 Kconfig/defconfig）
   CONFIG_SPL=y
   CONFIG_SPL_FRAMEWORK=y
   CONFIG_SPL_SERIAL_SUPPORT=y
   CONFIG_SPL_DRIVERS_MISC_SUPPORT=y
   ```

### 2.无SPL 

1. **BootROM 执行**
   SOC 上电后，BootROM 直接从外部存储加载完整 U-Boot 镜像到内存（一般是 DDR）并执行。
2. **U-Boot 阶段**
   U-Boot 自行初始化全部硬件（包含 DDR）。后续流程同上：提供 CLI、升级、内核引导等功能。
   
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
3. **操作系统启动**

> [!NOTE]
>
> 上述描述对应的是 **B 类情形**（BootROM 可直接加载完整镜像，或 NOR 直接映射），此时 U-Boot 确实自行完成 DDR 初始化。
>
> 但"无 SPL"另有两类同样不产出 `u-boot-spl.bin` 的情形：**A 厂商固件已代劳**（BL1 / BL2 完成 DDR 初始化，U-Boot 仅占 BL33 位置，如 Exynos4412）、**C `CONFIG_SPL` 未使能**（平台支持但配置未开）。
>
> 即：不能仅凭"编译产物中无 SPL"推断 SRAM 容量足够。详见前文「3.无 SPL › 更正与补充」。

## 5.板级初始化流程

> [!NOTE]
>
> 参考U-Boot源码根目录下的**README**。

### 1.整体流程概述

此流程板卡预设的启动流程，即硬件板卡正常启动时应遵循的标准步骤。该启动流程对两类程序均适用 ——**SPL（Secondary Program Loader，二级程序加载器）** 和 **U-Boot proper（标准 U-Boot 程序）**，且二者需遵循相同规则。

```c
//U-Boot（包括 SPL 和主 U-Boot）的板级初始化遵循统一规则，核心流程为：
架构相关的start.S → lowlevel_init() → board_init_f() → （BSS 清除 + U-Boot镜像重定位） → board_init_r() → main_loop()

//main_loop：uboot-主循环
//起点：从架构 / CPU 专属的start.S开始（如 ARMv7 的 `arch/arm/cpu/armv7/start.S`），这是汇编级别的启动入口。
//核心目标：逐步搭建硬件环境（从最基础的执行条件到完整的内存 / 外设可用），最终进入 U-Boot 主循环处理命令和启动系统。
```

### 2.BSS清除与重定位

在 `board_init_f()` 完成后、`board_init_r()` 执行前，会**自动**进行两项关键操作：

- **BSS 清除**
  清空未初始化的全局变量区域（确保全局变量初始值为 0）。
- **重定位**
  - **SPL**
    若定义 `CONFIG_SPL_STACK_R`，栈和 `global_data` 会被迁移到 `CONFIG_SPL_STACK_R_ADDR` 下方。
  - **主 U-Boot**
    整个 U-Boot 镜像被重定位到内存顶部。

### 3.三大核心函数

#### 1.lowlevel_init()

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

##### 新旧版本差异

旧版教程（U-Boot 1.1.6 ~ 2012.04，S3C2440 / mini2440）中，`lowlevel_init` 承担**时钟与 SDRAM 初始化**；而新版 README 规定 `must not set up SDRAM or use console`。二者并不冲突，差异源于**版本与启动框架的不同**。

| 维度                 | 老 U-Boot（S3C2440）      | 现代 U-Boot                              |
| -------------------- | ------------------------- | ---------------------------------------- |
| SPL                  | 无                        | 有，xPL 框架                             |
| SDRAM 初始化承担者   | `lowlevel_init`           | SPL / `board_init_f()` 的 `dram_init()`  |
| 代码运行位置         | Steppingstone 片内 4KB    | SPL 的 SRAM，随后 DDR                    |
| `lowlevel_init` 职责 | 时钟 + SDRAM + UART/NAND  | 基本为空函数                             |

**老平台将该职责置于此处的原因：** 没有 SPL，代码运行于片内 4KB Steppingstone，此时**无栈、无 `global_data`、BSS 未清**，且必须重定位到 SDRAM 才能继续执行。而 `cpu_init_crit` 的注释已明确指出：**内存时序与板级相关，必须在重定位之前完成配置**。因此 SDRAM 初始化被纳入 `lowlevel_init`，属平台约束下的必然结果，而非设计选择。

2012 年 MINI2440 合入主线时曾就此讨论：有意见提出将 SDRAM 初始化移至 `dram_init()` / `board_early_init_f()`，patch 作者以“重定位前必须完成时序配置、C 代码运行前必须存在栈”为由拒绝，最终维持原方案。部分 mini2440 版本改用 `CONFIG_SKIP_LOWLEVEL_INIT`（“We don't use lowlevel_init”）绕过该函数。

官方对在 `lowlevel_init` 中执行 SDRAM 初始化的定性，见 commit `db910353`。

#### 2.board_init_f()

1. **核心作用**
   为`board_init_r()`函数的运行做准备，重点初始化 SDRAM（同步动态随机存储器）和串行 UART（通用异步收发传输器），使硬件达到可运行后续初始化程序的状态。
2. **运行环境**
   - 可以访问`global_data`（全局数据结构GD，通常通过一个宏定义如 `DECLARE_GLOBAL_DATA_PTR`访问，本质上是一个指向 SRAM 中特定位置的指针，而不属于传统意义上的全局变量）
   - 栈（stack）位于 SRAM（静态随机存储器）中
   - BSS 段（未初始化的全局变量区域）不可用，因此不能使用全局变量或静态变量，只能使用栈变量和`global_data`
3. **无SPL 相关说明**
   - 会调用`dram_init()`来初始化 DRAM（动态随机存储器）
   - 若在 SPL（Secondary Program Loader，次级程序加载器）中已完成 DRAM 初始化，此函数可什么都不做
4. **SPL 相关说明**
   - 可根据需要用自定义版本覆盖整个`board_init_f()`函数
   - 在极端情况下可在此处调用`preloader_console_init()`
   - 需完成 SDRAM 初始化以及 UART 工作所需的所有设置
   - 无需清理 BSS 段，这部分工作会由`crt0.S`完成
   - 必须正常返回，不能直接调用`board_init_r()`

#### 3.board_init_r()

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

### 4.SPL与U-Boot 差异

|      维度      |                             SPL                              |                  主 U-Boot                   |
| :------------: | :----------------------------------------------------------: | :------------------------------------------: |
| **初始化流程** | 遵循相同的三阶段函数（`lowlevel_init` → `board_init_f` → `board_init_r`） |                     同上                     |
|  **代码路径**  |              通常有独立代码路径（`spl/` 目录）               |     主代码路径（`common/`、`board/` 等）     |
|  **核心目标**  |       快速初始化 SDRAM 和串口，加载主 U-Boot 或 Linux        | 完成完整初始化，提供命令行交互，启动操作系统 |
|  **资源限制**  |               体积小，依赖 SRAM 运行，功能精简               |  功能丰富，依赖 SDRAM 运行，可使用全局变量   |
|   **重定位**   |              栈和 `global_data` 可迁移到 SDRAM               |           整个镜像重定位到内存顶部           |

### 5.SPL 栈的迁移（CONFIG_SPL_STACK_R）

来源：commit `db910353`（Simon Glass，2015-03，committer Tom Rini）。

#### 1.背景

SPL 运行于 SRAM，可用栈空间很小（受 `CONFIG_SPL_STACK` / `CONFIG_SYS_INIT_SP_ADDR` 限制）。而部分 SPL 功能对栈的需求较大，典型如 **MMC 子系统**与 **environment**，栈空间不足会导致栈溢出。

旧做法：将 `CONFIG_SPL_STACK` 指向 SDRAM，并在 **`board_init_f()` 之前完成 SDRAM 初始化**，以便使用该处的大栈。其代价是 SDRAM 初始化被放入 `lowlevel_init()`。

#### 2.结论

Simon Glass 将其定性为对 `lowlevel_init()` 的滥用（abuse），commit message 原文：

> At present SPL uses a single stack, either CONFIG_SPL_STACK or CONFIG_SYS_INIT_SP_ADDR. Since some SPL features (such as MMC and environment) require a lot of stack, some boards set CONFIG_SPL_STACK to point into SDRAM. They then set up SDRAM very early, before board_init_f(), so that the larger stack can be used.
>
> This is an abuse of lowlevel_init(). That function should only be used for essential start-up code which cannot be delayed. An example of a valid use is when only part of the SPL code is visible/executable, and the SoC must be set up so that board_init_f() can be reached. It should not be used for SDRAM init, console init, etc.
>
> Add a CONFIG_SPL_STACK_R option, which allows the stack to be moved to a new address before board_init_r() is called in SPL.

要点两条：

1. `lowlevel_init()` 仅承担“使执行能够到达 `board_init_f()` 所必需、且不可延迟”的初始化。SDRAM 初始化、console 初始化均不属于此范畴。
2. 需要大栈时应采用下述方式：待 SDRAM 初始化完成后再迁移栈，无需提前配置 SDRAM。

#### 3.建议方法

新增两个 Kconfig 选项：

| 选项                | 作用                                              |
| ------------------- | ------------------------------------------------- |
| `SPL_STACK_R`       | 允许 SPL 在 `board_init_r()` 之前将栈迁移至新地址 |
| `SPL_STACK_R_ADDR`  | 指定栈在 SDRAM 中的地址                            |

关键时序：迁移发生在 **`board_init_f()` 完成之后、`board_init_r()` 执行之前**。因 SDRAM 正是在 `board_init_f()` 中完成初始化，待其就绪后再迁移栈即可，无需提前干预。

```
start.S / lowlevel_init()    仅完成最小初始化；不涉及 SDRAM / console；无栈、无 gd
        ↓
board_init_f()               初始化 SDRAM 与 UART；栈仍位于 SRAM；gd 可用
        ↓
[中间]  清 BSS
        CONFIG_SPL_STACK_R 定义时：栈与 global_data 迁移至 SPL_STACK_R_ADDR 之下
        ↓
board_init_r()               栈位于 SDRAM，MMC / env 可正常使用
```

README 对应原文：

> Here the BSS is cleared. For SPL, if CONFIG_SPL_STACK_R is defined, then at this point the stack and global_data are relocated to below CONFIG_SPL_STACK_R_ADDR.

#### 4.实现

`common/spl/spl.c` 新增 `spl_relocate_stack_gd()`：

```
ptr = CONFIG_SPL_STACK_R - sizeof(gd_t);
ptr &= ~7;                  /* 8 字节对齐，ABI 要求 */
将 global_data 复制到 new_gd；gd = new_gd;
清 BSS；
return 新栈位置；   /* 未定义 SPL_STACK_R 时返回 0，沿用原栈 */
```

`arch/arm/lib/crt0.S` 中的调用：

```asm
bl  spl_relocate_stack_gd
cmp r0, #0
movne sp, r0            /* 返回非 0 时切换栈指针 */
```

**返回 0 表示不迁移**，原有行为保持不变，因此该改动向后兼容，不影响既有板卡。后续补丁（2023）发现遗漏了 `gd->start_addr_sp` 的更新，需补充 `gd->start_addr_sp = ptr`，对齐方式亦改为 `roundup(sizeof(gd_t), 16)`。

#### 5.影响

该 commit 是 README 中 `lowlevel_init` 三条限制（不得配置 SDRAM / 使用 console、almost never needed）的**直接来源**。老 S3C2440 平台的做法与此规范相悖，但其没有 SPL、约束使然，最终因 `DM_SERIAL` 到期被移出 mainline。

# DDR初始化流程

## 1.初始化时机

DDR 尚未完成初始化时，CPU 没有可用的主存，只能运行在 SoC 内部容量有限的一小块 SRAM（OCRAM / iRAM）中。因此 DDR 的初始化必须发生在 SPL 阶段（在 `board_init_f()` 之前完成），由 SPL 完成 DDR 初始化后，再将完整版 U-Boot 加载到 DDR 中运行，最后跳转过去。

## 2.总体流程

从 DDR 控制器视角看，初始化可分为四个阶段：**时钟与控制器配置 → PHY 与 DRAM 上电 → 校准（训练）→ 验证与收尾**。

### 1.时钟与控制器配置

1. 配置 DDR PLL，产生 DRAM 工作时钟。
2. 复位 DDR 控制器与 PHY。
3. 配置控制器寄存器，包括：
   - 数据位宽、rank 数、bank 数、行 / 列地址数；
   - 时序参数（tRCD、tRP、tRAS、tRFC 等）；
   - 刷新周期。

### 2.PHY 与 DRAM 上电

1. 初始化 PHY（IO pad、阻抗、ZQ 引脚等）。
2. 按 JEDEC 规定执行 DRAM 上电序列：
   - DDR3：无复位引脚，拉低 CKE → 等待时钟稳定（约 500µs）→ 拉高 CKE；
   - DDR4 / DDR5：拉低 RESET_n → 上电稳定 → 拉高 RESET_n → 等待。
3. 执行 Precharge All（所有 bank 预充电）。
4. 写 MRS（Mode Register Set）模式寄存器，配置 CAS latency、burst length、DLL 复位等。

### 3.校准（训练）

1. ZQ 校准（ZQCL）。
2. 训练（Training）：
   - write leveling（写调平，DDR3 及以后）；
   - read DQS gate training（读 DQS 门控训练）；
   - 每字节 lane 的 read data eye 校准；
   - CA 训练（DDR4 / DDR5）。

> 训练的必要性：各颗颗粒、各根 DQ / DQS 信号到控制器的走线长度不同，存在相位差。低频下尚可容忍，高频下必须通过训练测出各 lane 的延时，将相位对齐，否则读写会出错。

### 4.验证与收尾

1. 内存测试（读写回读验证，或运行一段 memory test）。
2. 得出容量，写入 `gd->ram_size` / bd。
3. 重定位（relocate）U-Boot 到 DDR，之后全程在 DDR 中运行。

## 3.训练方式

| 方式         | 训练主体                                                         | 适用场景                             |
| ------------ | ---------------------------------------------------------------- | ------------------------------------ |
| 软件训练     | U-Boot SPL 内运行校准代码                                        | 早期 SoC（i.MX6/7、AM335x、早期全志） |
| DDR 固件训练 | 离线工具生成训练固件，SPL 加载固件，由固件完成 PHY 训练后返回寄存器值 | 现代 SoC（i.MX8/9、瑞芯微 RK35xx）   |

在 DDR 固件训练方式中，U-Boot 不再手写训练逻辑，只负责加载 DDR Firmware；训练参数由 DDR 工具离线计算后固化成表，U-Boot 仅做回放。这是当前趋势——高频 DDR 的训练逻辑复杂且厂商保密，通常打包成固件交给 Bootloader 调用。

## 4.与启动阶段的对应关系

DDR 初始化发生在 SPL 阶段，对应 ATF 体系的 BL2，详见下文“EL 与 BL”一节。

------

# U-Boot 重大更新

分界线有两条：

- **A 线：v2014.07 / v2014.10** —— 构建与配置系统重构（Kconfig + Kbuild），`boards.cfg` 被删除。
- **B 线：v2020.01** —— `CONFIG_DM` 对所有板强制启用，未迁移的板被移除。

2019 ~ 2023 年间各子系统陆续到期，其中 **`DM_SERIAL`（2023.04）是最晚的关键节点**。

| 版本              | 事件                                                         |
| ----------------- | ------------------------------------------------------------ |
| 2012              | SPL framework 出现（具体版本待考）                            |
| 2013.11           | Kbuild 风格 Makefile（Step 1）                                |
| 2014.02           | 真 Kbuild（Step 2）                                           |
| **2014.07 / 2014.10** | **Kconfig 落地，`boards.cfg` 删除（分界线 A）**            |
| 2015 初           | `arch/arm/mach-*` 目录重组（仿 Linux）                        |
| 2015              | `CONFIG_SPL_STACK_R`、`board_init_f_mem()`、`lowlevel_init` 瘦身 |
| 2019.04           | `DM_MMC` 到期                                                 |
| 2019.07           | `DM_USB` / `AHCI` / `BLK` / `DM_PCI` / `DM_VIDEO`             |
| **2020.01**       | **`CONFIG_DM` 全板强制（分界线 B）**                          |
| 2020.07           | `DM_ETH`                                                      |
| 2021.10           | `DM_I2C`                                                      |
| 2022.10           | `KEYBOARD`；`CONFIG_SYS_*` → `CFG_SYS_*` 大改名                |
| 2023.04           | `DM_SERIAL`（最晚的关键分界）                                 |

| 项目         | 老 U-Boot                            | 新 U-Boot                             |
| ------------ | ------------------------------------ | ------------------------------------- |
| 选板         | `make <board>_config`                | `make <board>_defconfig`              |
| 板定义       | `boards.cfg`                         | `arch/*/Kconfig` + `board/*/Kconfig`  |
| 宏来源       | `include/configs/<board>.h`          | defconfig + Kconfig                   |
| 构建         | 手写 Makefile                        | Kbuild（`scripts/Makefile.build`）    |
| 驱动注册     | 直接调函数                           | `U_BOOT_DRIVER()` + uclass            |
| 硬件参数     | 宏写死                               | 设备树 `.dts`                         |
| 批量编译     | `MAKEALL`                            | `buildman`                            |
| 维护信息     | 无                                   | 每板 `MAINTAINERS`                    |

**中间代（2014 ~ 2019）情况最复杂**：Kconfig 已引入，但 `CONFIG_DM` 尚未强制，两套 API 并存，同一源码树中部分板使用新机制、部分沿用旧机制。阅读代码时应统一按其中一套理解，避免混用。

**判断源码树所属年代的依据：**

```sh
ls boards.cfg             # 存在：2014.10 之前
ls configs/*_defconfig    # 存在：新版
grep -r U_BOOT_DRIVER drivers/ | head    # 存在：已使用 Driver Model
ls arch/arm/mach-*        # 存在：2015 之后
ls MAINTAINERS            # 每块板均有：2014.07 之后
```

**S3C2440 的现状：** `smdk2410` 已从 mainline 删除，删除理由原文为 “This board has not been converted to DM_SERIAL by the deadline”；`s3c24x0` 串口驱动一并删除（“not used by any boards”）；Linux 侧亦于 2023 年 1 月移除全部 s3c24xx 支持。因此旧版教程仅可作为启动流程的参考，无法直接用于新版 mainline U-Boot。

 
