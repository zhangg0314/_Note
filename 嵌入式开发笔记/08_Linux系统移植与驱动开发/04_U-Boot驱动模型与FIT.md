# U-Boot的DM

## 1. 概念

DM (Driver Model) 是 U-Boot 标准的 device-driver 开发模型，跟 kernel 的 device-driver 模型非常类似。

```txt
Terminology
-----------
Uclass - a group of devices which operate in the same way. A uclass provides
a way of accessing individual devices within the group, but always
using the same interface. For example a GPIO uclass provides
operations for get/set value. An I2C uclass may have 10 I2C ports,
4 with one driver, and 6 with another.
Driver - some code which talks to a peripheral and presents a higher-level
interface to it.
Device - an instance of a driver, tied to a particular port or peripheral.
```

## 2. 与Linux驱动的差异

U-Boot 和 Linux 内核在设备驱动初始化机制上的一个核心差异就是：**U-Boot 不会自动探测和初始化设备，必须由开发者在代码中显式调用 probe 函数来激活驱动**。具体可以从以下几个方面理解：

### 2.1 Linux 内核自动 probe

在 Linux 内核中，当系统启动时：

- 内核会先解析设备树，识别所有硬件设备
- 然后遍历所有已注册的驱动，通过`compatible`属性匹配设备和驱动
- 一旦匹配成功，内核会**自动调用驱动的 probe 函数**，完成设备初始化

整个过程是 “全自动” 的，开发者不需要手动编写代码触发 probe，内核框架会处理好一切。

### 2.2 U-Boot 手动 probe

在 U-Boot 中：

- 虽然也有设备树解析和驱动注册，但**不会自动执行 probe**

- 必须由开发者在初始化代码（如板级初始化函数）中，通过类似以下的方式**主动调用 probe**：

  ```c
  // 方式1：通过设备名获取设备并触发probe，该接口的核心调用就是device_probe
  uclass_get_device_by_name(UCLASS_I2C_GENERIC, "lt8912b", &dev);
  
  // 方式2：直接调用probe函数
  device_probe(dev);
  ```

- 只有执行了这些主动调用，驱动的 probe 函数才会被执行，设备才会被初始化

## 3. probe实现源码分析

`device_probe()` 是 U-Boot 设备模型中最核心的函数之一，负责完成设备的初始化工作。它的主要作用是将设备（`struct udevice`）与驱动（`struct driver`）绑定，并执行一系列初始化操作，最终调用驱动的 `probe` 函数激活设备。以下是逐段代码的详细解释：

```c
/*Activate a device so that it is ready for use. All its parents are probed first.*/
int device_probe(struct udevice *dev)
{	
//1.入参检查================================================================================================================
    const struct driver *drv;
	int size = 0,ret,seq;

	if (!dev)//非法参数检查
		return -EINVAL;

	if (dev->flags & DM_FLAG_ACTIVATED)//设备已激活
		return 0;

	drv = dev->driver;
	assert(drv);//判断传入的设置是否绑定与之匹配的驱动，即driver是否为空
//2.申请数据空间================================================================================================================
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
//3. 初始化父设备（依赖处理）================================================================================================================
	/* Ensure all parents are probed */
	if (dev->parent) {
        /*分配父设备的子设备私有数据（如果需要）*/
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
//4.设置设备序号与激活标志================================================================================================================

	/*获取设备在同类设备中的序号（如 i2c0、i2c1）*/
    seq = uclass_resolve_seq(dev);
	if (seq < 0) {
		ret = seq;
		goto fail;
	}
	dev->seq = seq;// 保存序号
	dev->flags |= DM_FLAG_ACTIVATED;// 标记设备开始激活（防止中途被重复初始化）
//5. 引脚配置与预处理================================================================================================================
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

    /* 父设备对当前子设备的 probe 前处理（如总线控制器准备子设备通信环境）,如果父设备的驱动定义了 child_pre_probe 方法（如 I2C 控制器为子设备设置通信参数），则调用该方法为当前设备做准备。*/
	if (dev->parent && dev->parent->driver->child_pre_probe) {
		ret = dev->parent->driver->child_pre_probe(dev);
		if (ret)
			goto fail;
	}
//6.设备树数据转换与驱动probe==============================================================================================================
	/* 将设备树数据转换为驱动的平台数据（platdata）,：如果驱动定义了 ofdata_to_platdata 方法（用于解析设备树属性），且设备有设备树节点，则调用该方法将设备树中的配置（如寄存器地址、时钟频率）转换为驱动可直接使用的平台数据（dev->platdata）。 */
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
//7.probe 后处理与完成================================================================================================================
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
    
//8.错误处理================================================================================================================ 
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

`device_probe()` 的核心逻辑可概括为：**检查状态 → 分配资源 → 初始化父设备 → 预处理（引脚、类、父设备） → 解析设备树 → 调用驱动 probe → 后处理**。整个过程确保了设备初始化的依赖顺序（父设备优先）、资源分配（私有数据）和硬件配置（引脚、设备树参数）的正确性，最终通过驱动的 `probe` 函数完成硬件激活，是 U-Boot 设备从 “定义” 到 “可用” 的关键桥梁。

------

# U-Boot的FIT

## 4. 核心实现原理

FIT 格式在不同架构芯片中实现硬件兼容性的核心逻辑是 **“用设备树语法描述‘镜像组件与硬件的匹配关系’，再通过 U-Boot 运行时的硬件检测动态选择适配组件”**。这种机制与芯片架构（如 ARM、PowerPC、RISC-V）无关，而是通过一套通用的 “兼容性标识 + 匹配算法” 实现跨架构适配。无论芯片架构是 ARM（如瑞芯微 RK3506）、PowerPC（如 NXP MPC85xx）还是 RISC-V，FIT 都通过以下机制保证兼容性：

### 4.1 组件标记

FIT 镜像中的每个组件（如 U-Boot 主程序、设备树）都通过 **“compatible” 属性** 标记它支持的硬件，格式与设备树中的 `compatible` 字段一致（这是跨架构兼容的关键）。例如：

- 一个支持 RK3506 评估板的 U-Boot 组件会标记：`compatible = "rockchip,rk3506-evb"`
- 一个支持 MPC8548 开发板的内核组件会标记：`compatible = "fsl,mpc8548ds"`

这些标签直接对应硬件的 “身份标识”，与架构无关。

### 4.2 硬件检测

U-Boot 的SPL启动时，会通过两种方式获取当前硬件的 “身份标识”：

- **读取芯片内置 ID**：如 ARM 芯片的 `CHIP_ID` 寄存器、PowerPC 的 `PVR`（处理器版本寄存器），确定芯片型号。
- **解析基础设备树**：对于复杂硬件，U-Boot 会先加载一个 “基础设备树”（通常固化在芯片或启动介质中），从中读取根节点的 `compatible` 属性（如 `compatible = "rockchip,rk3506"`），作为硬件的核心标识。

例如，RK3506 开发板的基础设备树根节点会声明：

```dts
/ {
    compatible = "rockchip,rk3506-evb", "rockchip,rk3506";
};
```

表示 “这是 RK3506 芯片的评估板”。

### 4.3 匹配算法

U-Boot 会将硬件的 “身份标识”（如 `rockchip,rk3506-evb`）与 FIT 镜像中组件的 `compatible` 标签进行比对，遵循 **“最长前缀匹配” 原则**：

- 优先选择与硬件标识完全一致的组件（如 `rockchip,rk3506-evb` 匹配标签相同的组件）。
- 若没有完全匹配，则匹配更通用的父级标识（如 `rockchip,rk3506` 匹配支持所有 RK3506 芯片的组件）。

这种算法与架构无关，仅依赖文本字符串比对，确保在任何架构下都能工作。

## 5. `.its`文件

`.its` 文件（Image Tree Source）是描述 FIT（Flattened Image Tree）镜像结构的**源文件**，采用类似设备树（DTS）的语法，用于定义 FIT 镜像包含的组件（如内核、设备树、固件）、配置信息和兼容性规则。最终通过 `mkimage` 工具编译为 `.itb` 二进制文件（FIT 镜像）。

### 5.1 文件的基本结构

```dts
/dts-v1/;  // 版本声明
/ {
    description = "FIT 镜像描述";  // 全局描述
    #address-cells = <1>;  // 地址字段的单元格数量（类似设备树）
    #size-cells = <1>;     // 大小字段的单元格数量

    images {  // 定义所有组件（镜像文件）
        // 组件1：如内核、U-Boot、设备树等
        image@1 {
            // 组件属性...
        };
        // 组件2...
    };

    configurations {  // 定义配置（组件组合方式）
        default = "conf@1";  // 默认配置
        // 配置1：指定使用哪些组件
        conf@1 {
            // 配置属性...
        };
        // 配置2...
    };
};
```

每个部分的作用：

- `images`：存放所有二进制组件（如内核、设备树、ramdisk），每个组件有唯一标识（如 `image@1`）。
- `configurations`：定义组件的组合方式（如 “用内核 A + 设备树 B 启动”），U-Boot 会根据硬件兼容性选择合适的配置。

### 5.2 核心语法与属性详解

#### 5.2.1 全局属性（根节点）

- `description`：FIT 镜像的描述文本（如 `"U-Boot FIT for RK3506"`），用户自定义，可以随便写。
- `#address-cells` 和 `#size-cells`：指定地址和大小字段的单元格数量（通常设为 `<1>` 或 `<2>`，与硬件地址宽度匹配）。

#### 5.2.2 定义组件（`images` 节点）

每个组件（如内核、设备树）是 `images` 的子节点，需指定以下关键属性：

|    属性名     |                             作用                             |                           示例值                            |
| :-----------: | :----------------------------------------------------------: | :---------------------------------------------------------: |
| `description` |                        组件的描述文本                        |              `"Linux kernel 5.10 for RK3506"`               |
|    `type`     |            组件类型（决定 U-Boot 如何处理该组件）            | `"kernel"`（内核）、`"fdt"`（设备树）、`"firmware"`（固件） |
|    `arch`     |                           目标架构                           |               `"arm"`、`"powerpc"`、`"riscv"`               |
|     `os`      |                   操作系统（仅对内核有效）                   |                          `"linux"`                          |
| `compression` |               压缩方式（`"none"` 表示未压缩）                |                     `"gzip"`、`"none"`                      |
|    `load`     | 加载地址（组件应被加载到的内存地址，`0x0` 表示由 U-Boot 自动分配，**设备树不需要**） |                        `0x80080000`                         |
|    `entry`    | 入口地址（组件的执行起始地址，内核 / 固件需要，**设备树不需要**） |                        `0x80080000`                         |
|    `data`     |    组件的二进制数据（通常通过 `incbin` 指令引用外部文件）    |             `incbin("zImage")` （引用内核文件）             |
|    `hash`     |             哈希校验（可选，用于验证组件完整性）             |                `sha256` { value = "xxx"; };                 |
| `compatible`  | 组件支持的硬件（用于兼容性匹配，类似设备树的 `compatible`）  |         `"rockchip,rk3506-evb", "rockchip,rk3506"`          |

#### 5.2.3 定义配置（`configurations` 节点）

配置节点用于组合 `images` 中的组件，告诉 U-Boot “如何使用这些组件启动”。每个配置需指定：

|    属性名     |                     作用                     |            示例值             |
| :-----------: | :------------------------------------------: | :---------------------------: |
| `description` |                配置的描述文本                | `"RK3506 EVB default config"` |
|   `kernel`    | 引用 `images` 中的内核组件（如 `kernel@1`）  |         `"kernel@1"`          |
|     `fdt`     |  引用 `images` 中的设备树组件（如 `fdt@1`）  |           `"fdt@1"`           |
|  `firmware`   | 引用 `images` 中的固件组件（如 U-Boot 镜像） |          `"uboot@1"`          |
| `compatible`  |   该配置支持的硬件（用于 U-Boot 选择配置）   |    `"rockchip,rk3506-evb"`    |

#### 5.2.4 哈希与签名（ 高级特性）

`.its` 支持为组件添加哈希校验或数字签名，用于验证完整性和安全性（需 U-Boot 开启 `CONFIG_FIT_SIGNATURE`）。

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

### 5.3 组件定义示例

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

### 5.4 定义默认配置示例

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

### 5.5 文件完整示例

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

### 5.6 编译 .its 为 .itb

通过 U-Boot 提供的 `mkimage` 工具将 `.its` 编译为 `.itb` 镜像：

```bash
mkimage -f example.its example.itb
```

- `-f`：指定 `.its` 源文件。
- 输出文件 `example.itb` 即为可被 U-Boot 识别的 FIT 镜像。

------

