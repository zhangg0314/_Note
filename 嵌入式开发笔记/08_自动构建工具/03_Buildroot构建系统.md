# Buildroot介绍

​	Buildroot是Linux平台上一个开源的嵌入式Linux系统自动构建框架。整个Buildroot是由Makefile脚本和Kconfig配置文件构成的。可以和编译Linux内核一样，通过 buildroot 配置，menuconfig修改，编译出一个完整的可以直接烧写到机器上运行的Linux系统软件(包含boot、kernel、rootfs以及rootfs中的各种库和应用程序)。

# 目录结构简介

## 1. 流程介绍

Buildroot 源码目录的结构设计非常清晰，每个目录和关键文件都有明确的分工，主要围绕 “配置 - 下载 - 编译 - 打包” 这一流程展开。

## 2. 目录结构

从官网[Buildroot - Making Embedded Linux Easy](https://buildroot.org/)下载源码包解压后目录结构如下:

```bash
.
├── arch
├── board
├── boot
├── CHANGES
├── Config.in
├── Config.in.legacy
├── configs
├── COPYING
├── DEVELOPERS
├── docs
├── fs
├── linux
├── Makefile
├── Makefile.legacy
├── package
├── README
├── support
├── system
├── toolchain
└── utils
```

## 3. 文件详细介绍

### 3.1 核心配置与入口文件

这些是使用Buildroot的起点，负责整体流程的控制和配置框架。

|    文件 / 目录     |                           作用说明                           |                           实例场景                           |
| :----------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
|     `Makefile`     | 顶层 Makefile，是所有 `make` 命令的入口，定义了构建流程（配置、编译、清理等）。 | 执行 `make menuconfig` 时，通过它加载配置界面；<br />执行 `make` 时，触发从下载源码到生成镜像的全流程。 |
|    `Config.in`     | 配置系统的顶层文件，递归包含所有子目录的 `Config.in`，形成完整的配置菜单。 | 它会引入 `package/Config.in`（软件包选项）、`linux/Config.in`（内核选项）等，最终在 `menuconfig` 中展示所有可配置项（如是否启用 Python、选择内核版本）。 |
| `Config.in.legacy` | 兼容旧版本配置选项的文件，处理配置项重命名或移除的兼容性问题。 | 若某个旧版本的配置项被改名，这里会定义映射规则，确保旧配置文件仍能正常加载。 |
| `Makefile.legacy`  | 兼容旧版本 `Makefile` 语法的兼容层，处理历史遗留的构建规则。 | 用于支持一些早期版本的自定义脚本，避免因 Buildroot 升级导致旧脚本失效。 |

### 3.2 硬件与平台相关目录

这些目录用于适配不同硬件平台，是嵌入式系统 “硬件相关” 部分的核心。

|   目录名   |                           作用说明                           |                           实例场景                           |
| :--------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
|  `arch/`   | 存放与 CPU 架构相关的配置（如 ARM、x86、MIPS 等），主要是工具链和内核的架构适配。 | `arch/Config.in` 中定义了 “Target Architecture” 选项，用户可选择 `arm`、`x86_64` 等架构，Buildroot 会据此选择对应的交叉编译器和内核配置。 |
|  `board/`  | 存放特定开发板的**定制化**文件（脚本、设备树、配置覆盖等），是硬件适配的关键。 | - `board/raspberrypi/`：树莓派的定制目录，包含启动脚本（`post-image.sh`）、根文件系统覆盖文件（如自定义`/etc/network/interfaces`）。 <br />- 当编译树莓派镜像时，Buildroot 会自动使用这里的文件适配硬件（如设置正确的分区表、加载对应设备树）。 |
| `configs/` | 预定义的硬件配置文件（`xxx_defconfig`），一键加载对应硬件的完整配置。 | - `configs/rockpi4_defconfig`：瑞芯微 Rock Pi 4 开发板的默认配置，包含适配该板的内核、U-Boot、文件系统类型等。 <br />- 使用 `make rockpi4_defconfig` 可直接初始化该开发板的配置，无需从零开始设置。 |

### 3.3 系统组件构建目录

这些目录负责构建嵌入式系统的核心组件：引导程序、内核、根文件系统等。

|  目录名   |                           作用说明                           |                           实例场景                           |
| :-------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
|  `boot/`  | 管理引导程序（如 U-Boot、GRUB）的下载、编译和集成，负责启动内核。 | - `boot/uboot/`：U-Boot 引导程序的配置目录，`uboot.mk` 定义了 U-Boot 的下载地址（如 `https://ftp.denx.de/pub/u-boot/u-boot-2023.10.tar.bz2`）、编译选项。 <br />- 若在 `menuconfig` 中启用 U-Boot，Buildroot 会自动编译并将其放入镜像的引导分区。 |
| `linux/`  | 管理 Linux 内核的下载、配置和编译，生成可在目标硬件运行的内核镜像。 | - `linux/linux.mk` 定义了内核源码的下载地址（如 `https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.50.tar.xz`）、配置文件路径。 <br />- 用户可通过 `menuconfig` 选择内核版本，或指定自定义内核配置文件（如 `board/xxx/linux.config`）。 |
|   `fs/`   | 将编译好的文件打包成各种格式的根文件系统镜像（如 ext4、squashfs 等）。 | - `fs/ext2/`：生成 ext4 格式镜像的规则，会调用 `mkfs.ext4` 工具将 `output/target/` 目录下的文件打包为 `rootfs.ext4`。 - `fs/squashfs/`：生成只读压缩的 squashfs 镜像，适合需要节省空间的场景（如路由器固件）。 |
| `system/` | 提供根文件系统的基础骨架（如 `/etc` 目录结构、初始化脚本），定义系统启动流程。 | - `system/skeleton/`：包含根文件系统的基础目录（`/bin`、`/lib`、`/etc` 等）和默认配置文件（如 `inittab` 初始化脚本）。<br /> - 这些文件会被复制到 `output/target/`，作为根文件系统的基础框架。 |

### 3.4 软件包与工具链目

这些目录控制软件包的编译和工具链的生成，是系统 “软件相关” 部分的核心。

|    目录名    |                           作用说明                           |                           实例场景                           |
| :----------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
|  `package/`  | 所有用户态软件包（如 BusyBox、Python、Nginx）的配置和编译规则。 | - `package/busybox/`：BusyBox 的配置目录，`busybox.mk` 定义了下载地址和编译选项，`Config.in` 提供 “是否启用 BusyBox”“是否包含特定命令（如 `ls`、`cd`）” 等选项。 <br />- 编译后，BusyBox 生成的工具会被安装到 `output/target/bin/`，成为系统的基础命令集。 |
| `toolchain/` | 管理交叉编译工具链的生成或使用（Buildroot 可自建工具链或使用外部工具链）。 | - `toolchain/Config.in` 中可选择 “使用 Buildroot 自建工具链”，并指定 C 库（如 musl、glibc）、编译器版本（如 GCC 12.x）。<br /> - 编译后，工具链会安装到 `output/host/bin/`（如 `arm-linux-gnueabihf-gcc`），用于编译其他软件包和内核。 |

### 3.5 辅助工具与文档

这些目录提供构建过程中的辅助功能和参考文档。

| 文件 / 目录  |                           作用说明                           |                           实例场景                           |
| :----------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
|  `support/`  | 存放构建过程中用到的辅助脚本和工具（如补丁管理、代码检查、测试工具）。 | - `support/scripts/download`：负责下载软件包源码，并校验 SHA256 哈希值（确保源码未被篡改）。<br /> - `support/testing/`：包含自动化测试脚本，验证 Buildroot 构建的系统是否正常工作。 |
|   `utils/`   |    提供实用工具脚本，辅助开发者处理配置、生成文件等任务。    | - `utils/config-stats`：分析当前配置文件（`output/.config`），统计启用的软件包数量和类型。 <br />- `utils/genrandconfig`：生成随机的配置文件，用于测试 Buildroot 的兼容性。 |
|   `docs/`    |     官方文档目录，包含用户手册、开发者指南、常见问题等。     | - `docs/manual/manual.html`：最常用的用户手册，详细解释如何配置 Buildroot、添加自定义软件包、适配新硬件等。 |
| `DEVELOPERS` | 维护者列表，记录每个软件包或功能的负责人（姓名、邮箱），方便社区协作。 |    若某个软件包出现问题，可通过该文件找到维护者寻求帮助。    |
|  `CHANGES`   | 版本变更记录，记录每个 Buildroot 版本的新功能、bug 修复和不兼容变更。 | 升级 Buildroot 前，可查看该文件了解是否有需要注意的兼容性问题。 |
|  `COPYING`   | 许可证文件，Buildroot 基于 GPLv2 协议发布，该文件详细说明开源许可条款。 |                              -                               |
|   `README`   | 入门指南，包含 Buildroot 的基本介绍、依赖安装、快速开始步骤（如首次编译的命令）。 |  新手可先阅读此文件，了解如何快速构建一个简单的嵌入式系统。  |

### 3.6 总结：从目录看 Buildroot 工作流程

1. **初始化配置**
   通过 `configs/xxx_defconfig` 或 `make menuconfig`（基于 `Config.in`）生成配置文件，确定目标架构、硬件平台、软件包等。
2. **构建工具链**
   `toolchain/` 目录生成或使用交叉工具链，为后续编译做准备。
3. **编译组件**
   - `boot/` 编译引导程序（如 U-Boot），`linux/` 编译内核。
   - `package/` 编译用户态软件（如 BusyBox、Python），安装到 `output/target/`。
4. **生成根文件系统**
   `fs/` 目录将 `output/target/` 打包成指定格式的镜像（如 ext4）。
5. **适配硬件**
   `board/` 目录的脚本和配置文件（如设备树、启动脚本）被集成到镜像，确保系统能在目标硬件上运行。

```bash
   1) run 'make menuconfig'
   2) select the target architecture and the packages you wish to compile
   3) run 'make'
   4) wait while it compiles
   5) find the kernel, bootloader, root filesystem, etc. in output/images
```

通过这些目录的协作，Buildroot 实现了 “一键式” 构建嵌入式 Linux 系统的目标，极大简化了嵌入式开发流程。



