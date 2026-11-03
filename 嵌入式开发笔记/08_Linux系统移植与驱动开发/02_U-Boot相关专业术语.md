# 背景介绍

U-Boot难看懂流程除了代码量大，还一个重要的原因就是有些术语概念不理解，导致看不懂代码，因此本笔记主要就是介绍一些本人对U-Boot代码分析时遇到的一些概念术语的理解。

# UBoot安全启动

## 1.安全引导

安全引导（Secure Boot）功能是指在系统的整个启动过程中，使用**链式**验证电子签名的方式来验证系统中重要镜像文件的可靠性，然后再加载镜像文件的引导过程。

安全引导功能可以保护二级厂商系统的独立性和完整性。安全引导功能的原理就是采用链式验签的方式启动系统，也就是在系统启动过程中，在加载下一个阶段的镜像之前都会对需要被加载的镜像文件进行电子验签，只有验签操作通过后，该镜像才能被加载到内存中，然后系统才会跳转到下一个阶段继续执行，整个验签链中的任何一环验签失败都会导致系统挂死，系统启动过程中的第一级验签操作是由Chip ROM来完成的。

## 2.安全启动原理

只要芯片一出厂，用户就无法修改固化在芯片中的这部分代码，因此无法通过修改第一级验签结果来关闭安全引导功能。而且验签操作使用的RSA公钥或者哈希值将会被保存在OTP（**一次性可编程存储器**）/efuse（**电子熔丝**）中，该区域中的数据一般只有Chip ROM和TEE能够读取且无法被修改。RSA公钥或者哈希值将会在产品出厂之前被写入到OTP/efuse中，而且不同厂商使用的密钥会不一样。

## 3.镜像验签流程

![image-20260907164537123](..\..\嵌入式开发笔记\figure\image-20260907164537123.png)

# ARM Trust Zone

## 1.基本概念

Trust Zone 是 ARM 为设备安全开发的一个**硬件架构**。它通过**硬件隔离的方式**将系统分为了两个环境——Non Secure 和 Secure ，也叫做 Normal 和 Trusted。

使用 TrustZone，在同一颗芯片里的 CPU、内存和外设，就可以按安全和非安全两种访问身份来管理。普通软件即使知道某个安全区域的地址，也不能直接访问它。这种架构类似于设置了一道由硬件检查的门：

```
普通系统（Non-secure）：Linux、普通应用
安全环境（Secure）：安全固件、可信执行环境中的程序
CPU 发起读写 → 总线携带 Secure/Non-secure 身份
             → 硬件检查目标地址是否允许访问
```

硬件隔离实现在两个方面：
- **外设隔离**
  例如把某个 UART 配成安全外设。普通系统即使知道它的寄存器地址，访问也会被硬件拦住；安全环境可以操作它。
- **内存隔离**
  例如从 DDR 中划出一段给安全环境存放密钥。`TZASC` 这类硬件负责检查对这段地址的访问。普通系统可以使用 DDR 的其他区域，却不能读写这段安全区域。

这里的把 UART、DDR 分成两个部分容易引起误解：不是把一颗 UART 物理切成两半，也不是必须把整块 DDR 对半分。通常是配置哪些外设或哪些内存地址范围属于安全区域。

例如：

Linux 需要加密服务，但密钥放在安全内存中。Linux 可以提出请用密钥加密这段数据的请求，由安全环境完成操作并返回结果；Linux 自己不能直接读取密钥。TrustZone 提供的就是这种隔离所需的硬件基础。

## 2.TZASC

TZASC是 **TrustZone Address Space Controller**，可以把它理解为 DDR 访问的硬件门卫。例如，芯片把 DDR 的一段地址配置为安全区域，用来放密钥：

```shell
DDR 地址范围
├─ 0x00000000～0x0FFFFFFF：普通区域
└─ 0x10000000～0x10FFFFFF：安全区域
```

CPU 或其他总线设备访问 DDR 时，请求会带有 **Secure 或 Non-secure** 身份。TZASC 检查访问的地址和请求的身份：Non-secure 请求若访问安全区域，就会被硬件拦住；Secure 请求则可按配置访问。

TZASC 通常是 **SoC（芯片）内部的独立硬件模块**，属于内存访问通路的一部分，**不是 CPU 核心内部的部件，也不是 UART 那样供软件收发数据的外设**。可以这样理解它的位置：

```
CPU / DMA → 芯片内部总线 → TZASC 权限检查 → DDR 控制器 → DDR
```

CPU 或 DMA 发起读写时，请求带着 Secure / Non-secure 身份；TZASC 根据目标 DDR 地址和预先配置的规则，决定是否放行。具体芯片中它也可能与 DDR 控制器或总线安全模块集成，位置要以该芯片的硬件手册为准。

# ARM Trusted Firmware

## 1.引入背景

在Armv8中，TF(Trusted Firmware)是ARM引入的**安全解决方案**，它的核心功能为对启动和运行过程进行特权级划分，为安全提供了整体解决方案。

相较于Armv7中的Trust Zone（TZ）进行了提高，它包括启动和运行过程中的特权级划分，**实现了启动过程信任链的传导，细化了运行过程的特权级区间**。

TF实际有两种Profile，对ARM Profile A的CPU应用TF-A，对ARM Profile M的CPU应用TF-M。我们一般接触的都是TF-A，又因为这个概念是ARM提出的，有时候也缩写做ATF（ARM Trusted Firmware）。

## 2.异常级别

EL = **Exception Level**（异常级别）。CPU 上电启动或遇异常/中断时，会在不同特权层切换，这些层就叫异常级别。ARM64 分 4 层，数字越大权限越高：

```txt
EL0  应用程序层   ← Linux 用户程序跑这
EL1  内核层       ← Linux 内核跑这
EL2  Hypervisor   ← 虚拟机管理层，没虚拟化就跳过
EL3  最高层       ← 安全监控，ATF/BL31 跑这，管 Secure/Normal 切换
```

启动顺序：**EL3（上电）→ 往下掉到 EL1 → EL0**。每层能管下面，下面碰不到上面。

再加两个维度：

- Secure World 里 EL1 = **S-EL1**，跑 TEE OS（OP-TEE）。
- Normal World 里 EL1 = **NS-EL1**，跑 Linux。

![img](D:\MyNote\-\嵌入式开发笔记\figure\2207466-20250119154916325-237413260.png)

## 3.ATF 固件代号

ARM Trusted Firmware（ATF）把启动分成几个阶段，按 **BL（Boot Loader）+ 数字**命名

![img](D:\MyNote\-\嵌入式开发笔记\figure\2207466-20250119160956027-82102496.png)

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

# 不同ARM架构安全策略

## 1.ARMv7

ARMv7 有 EL 概念但没有 EL3（沿用 old-style 特权模式），也没有 ATF 标准。可信执行环境就是 TOS（Trusted OS = OP-TEE） 本身。所以在`RockChip`的uboot编译脚本里可以看到：

```shell
  if [ "${ARM64_TRUSTZONE}" == "y" ]; then
      ${SCRIPT_ATF} --ini ${INI_TRUST} ...   # ARM64：ATF，产出 bl31 → trust.img，bl31 = Boot Loader 阶段 3 的第 1 个组件，正好是常驻 EL3 的安全监控固件
  else
      ${SCRIPT_TOS} --ini ${INI_TRUST} ...   # ARM32：直接打包 TOS → trust.img，ARM32 没 EL3/ATF，安全固件直接是 TOS。
  fi
```

## 2.ARMv8

 ARMv8-A 是架构名，它定义两种执行状态：

  - AArch64 —— 64 位
  - AArch32 —— 32 位

同一个 ARMv8 核，可以跑 AArch64，也可以跑 AArch32（向后兼容 ARMv7 指令）。所以 ARM32 完全可以是 ARMv8——只是它跑在AArch32 状态。

|     情况     |     核     | trust镜像 |
| :----------: | :--------: | :-------: |
|   纯 ARMv7   | 只有 32 位 |    TOS    |
| ARMv8 + 64位 |  AArch64   |    ATF    |
| ARMv8 + 32位 |  AArch32   |    ATF    |

 关键：ARMv8 核有 EL3 / TrustZone，AArch64、AArch32 状态都有。ARMv7 核没有 EL3。

## 3.异同总结

两种架构的安全启动都遵循下列流程：

```bash
BootROM → loader(带DDR init) → trust.img → uboot → kernel

#loader 把 trust.img 搬进安全内存，再跳 U-Boot。U-Boot 要干安全相关的活（开副核、复位、进 TEE）就发 SMC指令陷进安全世界，由 trust 那块处理。
```

 两者差别在「安全世界」：

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
