- 文档版本：1.0
- 适用范围：Rockchip SDK 分发的 U-Boot 源码树（含 `configs/rk3506_defconfig`、`arch/arm/mach-rockchip/rk3506/`、`board/rockchip/evb_rk3506/`）
- 内容范围：配置阶段（`make rk3506_defconfig`）、编译阶段（TPL/SPL/U-Boot 本体）、打包阶段（`idblock.bin`/`uboot.img`）的完整链路，以及 Kconfig 配置系统与 kbuild 构建机制的说明
- 文中所有文件路径与行号均以该源码树为准

---

# 基本概述

RK3506 U-Boot 的构建分为三个阶段：

1. **配置阶段**
   执行 `make rk3506_defconfig`，根据 `configs/rk3506_defconfig` 与全树 Kconfig 规则生成 `.config` 及后续派生文件。
2. **编译阶段**
   执行 `make all`，依次构建主机工具、TPL、SPL、U-Boot 本体与设备树。
3. **打包阶段**
   由 `make.sh` 调用 `scripts/fit.sh`，将各阶段产物封装为烧录镜像。

最终产物链：

```shell
tpl/u-boot-tpl.bin + spl/u-boot-spl.bin  →  idblock.bin（引导镜像）
u-boot-nodtb.bin + u-boot.dtb + tee.bin  →  uboot.img（FIT 格式，含 OP-TEE）
```

由于 `configs/rk3506_defconfig` 中 `CONFIG_ROCKCHIP_FIT_IMAGE_PACK=y`，`make.sh` 的 `PLAT_TYPE` 取值为 `FIT`，打包走 FIT 流程。

---

# 核心概念

## 1.Kconfig

Kconfig 是 U-Boot（继承自 Linux 内核）的配置描述系统，包含一套声**明式配置语言**与一个**解析程序**。

- **Kconfig 文件**
  遍布源码树，每个目录一个，通过 `source` 指令互相包含，构成一棵配置树。
  语言要素包括 `config`（选项定义）、`bool`/`int`/`hex`/`string`（类型）、`default`（默认值）、`select`（强依赖）、`imply`（弱依赖）、`depends on`（前置条件）、`menu`/`choice`（界面结构）。
- **解析程序**
  `scripts/kconfig/conf`，由 `conf.c` 与词法/语法解析器构成，提供 `--defconfig`、`--silentoldconfig`、`--savedefconfig` 等模式；`menuconfig`/`nconfig`/`xconfig`/`gconfig` 为交互式前端。

Kconfig 语言的完整语法参考见第 9 章。

## 2.config 与派生文件

`.config` 是 conf 程序合并"Kconfig 规则 + 所选 defconfig"后写出的配置结果文件，为纯文本键值对，四种形式：

```.conf
CONFIG_ARM=y                          # bool 打开
CONFIG_DEBUG_UART_BASE=0xff0a0000     # hex
CONFIG_USB_GADGET_MANUFACTURER="Rockchip"  # string
# CONFIG_CMD_NAND is not set          # bool 关闭
```

`.config` 规模（上千行）远大于 `configs/rk3506_defconfig`（171 行）：后者只记录显式差异，其余选项在配置过程中按 Kconfig 的 `default` 补齐。

`.config` 为中间产物，编译与链接实际消费下列派生文件：

| 文件 | 生成方式 | 用途 |
|:-:|:-:|:-:|
| `include/config/auto.conf` | `conf --silentoldconfig` | makefile 语法，被顶层 Makefile `-include` |
| `include/generated/autoconf.h` | 同上 | C 头文件，经 `-include include/linux/kconfig.h` 注入所有源文件 |
| `include/config.h` | `scripts/Makefile.autoconf` | 板级配置入口，内容含 `#include <configs/evb_rk3506.h>` |
| `include/autoconf.mk` 及 `spl|tpl/include/autoconf.mk` | 对 `include/common.h` 预处理提取 CONFIG 宏 | 老式 mk 语法配置（双轨制过渡产物） |

## 3.主机工具与交叉工具链

| 类别 | 编译器 | 运行环境 | 引入阶段 |
|:-:|:-:|:-:|:-:|
| 主机工具（`conf`、`fixdep`、`mkimage`、`fdtgrep`、`dtc`） | 宿主 gcc（HOSTCC） | 构建机 | 配置/打包阶段，自动构建 |
| 交叉工具链（`arm-linux-gnueabihf-gcc`） | 不编译 | 构建机（生成 ARM 代码） | 仅 `make all` 编译目标代码时 |

主机工具由构建系统自动构建，无需预先手动编译。

## 4.make 机制

- **模式规则（pattern rule）**
  目标名中含 `%` 通配符的规则，目标名匹配时 `%` 捕获的部分称为 stem。如 `%config` 匹配 `rk3506_defconfig`（stem 为 `rk3506_def`），`%_defconfig` 匹配 `rk3506_defconfig`（stem 为 `rk3506`）。
- **include 指令**
  make 解析期的文件包含指令，将被包含文件的规则与变量并入当前解析流，与 C 语言 `#include` 语义一致。
- **递归 make**
  在规则命令体中调用新的 make 进程，子进程拥有**独立的**规则库与变量环境。
- **FORCE 与伪目标**
  FORCE 为无命令的恒过期目标，保证依赖它的规则每次执行；
  伪目标（.PHONY）不检查文件时间戳。

---

# 编译链路总览

### 3.1 参与编译的目录分层

按"通用 / 平台 / SoC / 板级"四层划分，门控机制为各目录 Makefile 中的 `obj-$(CONFIG_XXX) += foo.o`，由 `scripts/Makefile.build` 将满足条件的源文件编入 `built-in.o`。

| 层次 | 目录/文件 | 门控 |
|:-:|---|---|
| 通用基础 | `lib/` `common/` `cmd/` `env/` `fs/` `net/` `disk/` `drivers/` `test/`；`arch/arm/cpu/armv7/` `arch/arm/cpu/` `arch/arm/lib/`；`scripts/` `tools/` | 无条件 `obj-y` 或对应 CONFIG |
| 瑞芯微平台 | `arch/arm/mach-rockchip/`（顶层文件，不含 rk3506/ 子目录）；`include/configs/rockchip-common.h`；`drivers/clk/rockchip/` `drivers/pinctrl/rockchip/` `drivers/ram/rockchip/` 等；`make.sh`、`scripts/fit.sh` | `CONFIG_ARCH_ROCKCHIP` |
| RK3506 SoC | `arch/arm/mach-rockchip/rk3506/`（`rk3506.c`、`syscon_rk3506.c`）；`arch/arm/include/asm/arch-rockchip/cru_rk3506.h` `grf_rk3506.h` `ioc_rk3506.h`；`drivers/clk/rockchip/clk_rk3506.c`；`drivers/pinctrl/rockchip/pinctrl-rk3506.c`；`drivers/ram/rockchip/sdram_rk3506.c`；`drivers/misc/rk3506-secure-otp.S`；`arch/arm/dts/rk3506*.dts/i`；`include/configs/rk3506_common.h`；`configs/rk3506_defconfig` | `CONFIG_ROCKCHIP_RK3506` |
| 板级 | `board/rockchip/evb_rk3506/`（`evb_rk3506.c`、`Kconfig`、`Makefile`）；`include/configs/evb_rk3506.h` | `CONFIG_TARGET_EVB_RK3506` |

`arch/arm/Makefile` 第 74 行将 `CONFIG_ARCH_ROCKCHIP` 映射到机器目录 `rockchip`；`arch/arm/mach-rockchip/Makefile` 第 77 行按 `CONFIG_ROCKCHIP_RK3506` 引入 `rk3506/` 子目录。

### 3.2 三阶段编译

| 阶段 | ELF | 二进制 | 链接脚本 | 基址 | 说明 |
|---|---|---|---|---|---|
| TPL | `tpl/u-boot-tpl` | `tpl/u-boot-tpl.bin`（44 KB） | `arch/arm/mach-rockchip/u-boot-tpl.lds`（`TPL_LDSCRIPT`，Kconfig 第 406 行） | 0xfff81000（SRAM） | `TPL_TINY_FRAMEWORK=y`，主框架为 `arch/arm/mach-rockchip/tpl.c` |
| SPL | `spl/u-boot-spl` | `spl/u-boot-spl.bin`（256 KB，含裁剪 DTB） | `arch/arm/cpu/u-boot-spl.lds`（回退链终点） | 0x03f00000（DDR） | `common/spl/` 框架，`CONFIG_SPL_FRAMEWORK=y` |
| U-Boot | `u-boot` | `u-boot-nodtb.bin` + `u-boot.dtb` → `u-boot.bin` | `arch/arm/cpu/u-boot.lds`（回退链终点） | 0x00200000（DDR） | 主链接命令见顶层 Makefile 第 1290 行 |

链接脚本回退顺序：板级目录 → `$(CPUDIR)`（`arch/arm/cpu/armv7/`）→ `arch/arm/cpu/`。TPL 的链接脚本由 Kconfig 显式指定，不经过回退链。

启动入口 `head-y` 为 `arch/arm/cpu/armv7/start.o`（`arch/arm/Makefile` 第 86 行），必须位于链接顺序首位。

### 3.3 设备树编译链

1. `arch/arm/dts/rk3506-evb.dts`（`#include "rk3506.dtsi"` 与 `"rk3506-u-boot.dtsi"`）经 `scripts/Makefile.lib` 的 `%.dtb: %.dts` 规则由 dtc 编出 `rk3506-evb.dtb`。
2. 顶层 `dts/Makefile` 依据 `CONFIG_DEFAULT_DEVICE_TREE="rk3506-evb"`，生成：
   - `dts/dt.dtb`：完整 DTB，供 U-Boot 本体使用；
   - `dts/dt-spl.dtb`：仅保留含 `u-boot,dm-spl` 属性的节点（fdtgrep 裁剪）；
   - `dts/dt-tpl.dtb`：仅保留含 `u-boot,dm-tpl` 属性的节点。
3. 产物：`u-boot.dtb`、`u-boot-dtb.bin`（`u-boot-nodtb.bin` 拼接 DTB）；SPL 侧为 `u-boot-spl-dtb.bin`。

`rk3506-u-boot.dtsi` 中的 `u-boot,dm-spl` / `u-boot,dm-pre-reloc` 属性决定 SPL 阶段可用的外设（mmc、cru、grf、pinctrl、crypto、rng、saradc、fspi、usb2phy 等）。

### 3.4 built-in.o 的生成机制

"由 `scripts/Makefile.build` 将满足条件的源文件编入 `built-in.o`"具体由六个环节构成，以 `drivers/clk/rockchip/Makefile` 的 `obj-$(CONFIG_ROCKCHIP_RK3506) += clk_rk3506.o` 为例：

**环节 1 条件展开**：`obj-$(CONFIG_X) += foo.o` 中的 `$(CONFIG_X)` 取值为 `include/config/auto.conf` 中的配置结果（`scripts/Makefile.build` 第 47 行 `-include include/config/auto.conf`）。值为 `y` 时行展开为 `obj-y += ...`（编入）；值为空时展开为 `obj- += ...`（丢弃）。

**环节 2 obj-y 预处理**（`scripts/Makefile.lib` 第 41~72 行）：

- 第 41~42 行：目录项 `foo/` 记入 `subdir-y`，供递归下降；
- 第 45 行：`obj-y` 中的目录项改写为 `foo/built-in.o`；
- 第 53 行：存在 `foo-objs` 的复合对象判定（`multi-used-y`）；
- 第 72 行：`real-objs-y` 为本目录实际编译的 `.o` 列表；
- 第 81 行：全部加上 `$(obj)/` 前缀。

**环节 3 built-in.o 成为构建目标**（`scripts/Makefile.build` 第 108~110 行）：本目录 `obj-y` 非空时定义 `builtin-target := $(obj)/built-in.o`；第 116~118 行 `__build` 将其列入必须构建的目标。

**环节 4 链接命令**（`scripts/Makefile.build` 第 350~361 行）：对 `obj-y` 中的全部 `.o` 执行 `ld -r`（可重定位链接，只合并目标文件、不解析符号），合并为本目录 `built-in.o`；本目录无自有对象时创建空 `built-in.o`（第 356 行）。

**环节 5 递归下降**（`scripts/Makefile.build` 第 423~425 行）：`subdir-ym` 中每个目录触发一次递归 make（`$(MAKE) $(build)=$@`），子目录重复环节 1~4 产出各自的 built-in.o，逐级向上合并；第 345 行保证父目录 built-in.o 依赖子目录 built-in.o 先建好。

**环节 6 顶层链接**（顶层 Makefile 第 725、1277~1290 行）：`libs-y` 全部改写为 `built-in.o` 后作为 `u-boot-main`，最终以 `ld --start-group $(u-boot-main) --end-group` 链接出 `u-boot` ELF。

```
obj-$(CONFIG_X) += foo.o          环节 1  条件展开（auto.conf 驱动）
        │
        ▼
obj-y → real-objs-y               环节 2  Makefile.lib:72 筛选
        │
        ▼
$(obj)/%.o ← $(src)/%.c          环节 3/4 逐个编译（CC，Makefile.build:279）
        │
        ▼
ld -r 合并 → $(obj)/built-in.o   环节 4  Makefile.build:353-358
        │
        ▼ 递归（subdir-ym，Makefile.build:424）逐级上并
        ▼
顶层 ld --start-group 全部 built-in.o → u-boot   环节 6  Makefile:1277-1290
```

SPL/TPL 复用同一套机制：`scripts/Makefile.spl` 第 101 行同样将 `libs-y` 改写为 built-in.o，仅增加 `-DCONFIG_SPL_BUILD` 编译标志，使 `obj-spl-y` 与 `obj-$(CONFIG_SPL_*)` 生效。

---

# 配置阶段

## 1.顶层路由

顶层 Makefile 第 438~455 行判断 `MAKECMDGOALS`：`rk3506_defconfig` 命中 `$(filter config %config,...)`，置 `config-targets=1`，进入第 474 行 `ifeq ($(config-targets),1)` 分支。第 485~486 行给出规则：

```makefile
%config: scripts_basic outputmakefile FORCE
	$(Q)$(MAKE) $(build)=scripts/kconfig $@
```

其中 `build := -f $(srctree)/scripts/Makefile.build obj`（**`scripts/Kbuild.include` 第 182 行**），展开后为：

```shell
make -f scripts/Makefile.build obj=scripts/kconfig rk3506_defconfig
```

前置条件语义：

- `FORCE`
  恒过期，保证命令体每次执行；
- `scripts_basic`（第 404 行，PHONY）
  编出 `scripts/basic/fixdep`，后续所有 `if_changed_dep`（含宿主对象编译）依赖它生成 `.cmd` 文件；
- `outputmakefile`（第 412 行）
  仅在使用 `O=` 输出目录时生效，默认空操作。

## 2.conf 的自动构建

`%_defconfig` 规则位于 `scripts/kconfig/Makefile` 第 120~121 行：

```makefile
%_defconfig: $(obj)/conf
	$(Q)$< $(silent) --defconfig=arch/$(SRCARCH)/configs/$@ $(Kconfig)
```

其中 `SRCARCH := ..`（第 15 行），实际命令为 `conf --defconfig=../configs/rk3506_defconfig Kconfig`。

`conf` 为 `hostprogs-y`（第 204 行）成员，其构建由通用宿主规则完成：

- `conf-objs := conf.o zconf.tab.o`（第 196 行）；
- 编译：`scripts/Makefile.host` 第 113~116 行（HOSTCC `-c`）；
- 链接：`scripts/Makefile.host` 第 103~108 行（HOSTLD）；
- 原料：`zconf.tab.c` 由 `zconf.tab.c_shipped` 经 `scripts/Makefile.lib` 第 249~252 行的 `SHIPPED` 规则复制生成（`zconf.lex.c`、`zconf.hash.c` 同理）。

该过程只使用宿主 gcc，不涉及交叉工具链，也不需要 bison/flex/gperf（解析器代码已预生成并随仓库分发）。

注：`scripts/kconfig/Makefile` 第 216 行 `always := dochecklxdialog` 使每次进入 scripts/kconfig 目录都会执行 ncurses 检查（`lxdialog/check-lxdialog.sh -check`），构建环境需具备 ncurses 开发库。

**conf 的生成规则分布**

`$(obj)/conf`（即 `scripts/kconfig/conf`）的生成规则分布在四个文件中：

| 环节 | 位置 | 内容 |
|---|---|---|
| 配料 | `scripts/kconfig/Makefile:196,204` | `conf-objs := conf.o zconf.tab.o`；`hostprogs-y := conf ...` |
| 分类与前缀 | `scripts/Makefile.host:26,34-35,42,60-63` | conf 归入 `host-cmulti`；conf.o/zconf.tab.o 归入 `host-cobjs`；加 `$(obj)/` 前缀 |
| 链接规则 | `scripts/Makefile.host:103-109` | `$(host-cmulti): FORCE`，命令 `HOSTLD` |
| 编译规则 | `scripts/Makefile.host:113-116` | `$(host-cobjs): $(obj)/%.o: $(src)/%.c FORCE`，命令 `HOSTCC -c` |
| 依赖注入 | `scripts/Makefile.lib:198-202`（`multi_depend`） | 第 109 行调用，展开出 `conf: conf.o zconf.tab.o` |
| 原料生成 | `scripts/Makefile.lib:249-252` | `$(obj)/%: $(src)/%_shipped`，`cat` 复制出 zconf.tab.c 等 |
| 触发 | `scripts/kconfig/Makefile:120-121` | `%_defconfig: $(obj)/conf` |

展开后的实际规则（代入具体文件名）：

```makefile
# 链接（显式规则，host-cmulti 展开 + multi_depend 注入依赖）
scripts/kconfig/conf: scripts/kconfig/conf.o scripts/kconfig/zconf.tab.o FORCE
	$(HOSTCC) $(HOSTLDFLAGS) -o scripts/kconfig/conf \
		scripts/kconfig/conf.o scripts/kconfig/zconf.tab.o

# 编译（静态模式规则）
scripts/kconfig/conf.o scripts/kconfig/zconf.tab.o: scripts/kconfig/%.o: scripts/kconfig/%.c FORCE
	$(HOSTCC) $(hostc_flags) -c -o $@ $<    # hostc_flags 含 -Wp,-MD,<depfile>

# 额外依赖（scripts/kconfig/Makefile:296）
scripts/kconfig/zconf.tab.o: scripts/kconfig/zconf.lex.c scripts/kconfig/zconf.hash.c
```

规则链图：

```
%_defconfig: scripts/kconfig/conf                       kconfig/Makefile:120
  │
  ▼ 显式规则（Makefile.host:107 + multi_depend）
scripts/kconfig/conf: conf.o zconf.tab.o FORCE
  │  cmd: HOSTCC $(HOSTLDFLAGS) -o conf conf.o zconf.tab.o
  │
  ▼ 静态模式规则（Makefile.host:115）
scripts/kconfig/%.o: scripts/kconfig/%.c FORCE
  │  cmd: HOSTCC -c -o $@ $<          # if_changed_dep → 调用 scripts/basic/fixdep
  │
  ├─ conf.o      ← scripts/kconfig/conf.c
  ├─ zconf.tab.o ← zconf.tab.c（另依赖 zconf.lex.c、zconf.hash.c）
  │
  ▼ SHIPPED 规则（Makefile.lib:252）
zconf.tab.c  ← zconf.tab.c_shipped（cat，零编译）
zconf.lex.c  ← zconf.lex.c_shipped
zconf.hash.c ← zconf.hash.c_shipped
```

## 3.conf 执行流程

`scripts/kconfig/conf.c` 的 `main()` 按序执行：

| 步骤 | 代码位置 | 动作 |
|---|---|---|
| 1 | `conf_parse(name)`（第 565 行） | 解析整棵 Kconfig 树：顶层 `Kconfig` → `arch/Kconfig` → `arch/arm/Kconfig` → `arch/arm/mach-rockchip/Kconfig` → `arch/arm/mach-rockchip/rk3506/Kconfig` → `board/rockchip/evb_rk3506/Kconfig` 及 `drivers/Kconfig`、`lib/Kconfig` 等 |
| 2 | `conf_read(defconfig_file)`（第 584 行） | 读入 `configs/rk3506_defconfig`（171 行） |
| 3 | `conf_set_all_new_symbols(def_default)`（第 665~666 行） | 未显式设置的符号按 `default` 补齐；处理 `select`/`imply`/`depends on`。例如 `ROCKCHIP_RK3506 select CPU_V7` 使 `.config` 出现 `CONFIG_CPU_V7=y`；`imply SPL/TPL` 使 `.config` 出现 `CONFIG_SPL=y`、`CONFIG_TPL=y`、`CONFIG_TPL_TINY_FRAMEWORK=y` |
| 4 | `conf_write(NULL)`（第 707~711 行） | 写出 `.config`，并备份 `.config.old` |

`--defconfig` 模式不生成 `auto.conf`/`autoconf.h`；这些文件在后续 `make all` 阶段由 `--silentoldconfig` 生成。

## 4.后续配置链（make all 阶段）

1. 顶层 Makefile 第 515 行检测到 `.config` 新于 `include/config/auto.conf`，自动执行 `make silentoldconfig`，生成 `include/config/auto.conf` 与 `include/generated/autoconf.h`。
2. `scripts/Makefile.autoconf` 生成 `include/config.h`（第 102~114 行模板，含 `#include <configs/evb_rk3506.h>`，由 `CONFIG_SYS_CONFIG_NAME` 决定）与 `include/autoconf.mk`。
3. `create_symlink`（第 124~143 行）建立 `arch/arm/include/asm/arch` → `arch-rockchip` 符号链接（`CONFIG_ARM select CREATE_ARCH_SYMLINK`，见 `arch/Kconfig` 第 20~22 行）。

配置头文件链：`include/configs/evb_rk3506.h` → `include/configs/rk3506_common.h` → `include/configs/rockchip-common.h`。`rk3506_common.h` 定义 `CONFIG_SYS_TEXT_BASE`（0x00200000）、`CONFIG_SPL_TEXT_BASE`（0x03f00000）、SPL 尺寸上限、OTP 布局、env 布局等。

---

## 5. 构建系统自举机制

构建系统遵循"目标自备工具"原则：任何配置或编译目标所需的主机工具均作为该目标的依赖自动构建，无需预先手动编译。

```
make rk3506_defconfig
 ├─ scripts/basic/fixdep          自动构建（顶层 Makefile 的 scripts_basic）
 ├─ scripts/kconfig/conf          自动构建（%_defconfig 的前置依赖）
 └─ conf --defconfig=... Kconfig  生成 .config

make all
 ├─ scripts/basic/fixdep          已存在
 ├─ scripts/dtc/dtc               自动构建（设备树编译器）
 ├─ tools/mkimage、tools/fdtgrep  自动构建
 └─ 交叉编译 tpl/spl/u-boot 源码
```

配置阶段所需环境：`make`、宿主 C 编译器（gcc）、ncurses 开发库。交叉工具链、python 等仅在编译目标代码与打包阶段引入。

---

## 6. 目标依赖链解析（以 rk3506_defconfig 为目标）

### 6.1 依赖树

```
make rk3506_defconfig
│
├─[目标] scripts_basic
│   └─ make -f scripts/Makefile.build obj=scripts/basic
│       └─ fixdep ← scripts/basic/fixdep.c        （HOSTCC 编译）
│
├─[目标] outputmakefile                             （无 O= 时空操作）
├─[目标] FORCE                                      （恒过期）
│
└─ make -f scripts/Makefile.build obj=scripts/kconfig rk3506_defconfig
    │
    ├─[目标] scripts/kconfig/conf
    │   ├─ conf.o    ← scripts/kconfig/conf.c      （HOSTCC -c）
    │   └─ zconf.tab.o ← zconf.tab.c               （HOSTCC -c）
    │       ├─ zconf.tab.c  ← zconf.tab.c_shipped  （SHIPPED：cat）
    │       ├─ zconf.lex.c  ← zconf.lex.c_shipped  （SHIPPED）
    │       └─ zconf.hash.c ← zconf.hash.c_shipped （SHIPPED）
    │   （conf.o / zconf.tab.o 的依赖追踪调用 scripts/basic/fixdep）
    │
    └─[执行] conf --defconfig=../configs/rk3506_defconfig Kconfig
        ├─ 读 Kconfig 树（只读）
        ├─ 读 configs/rk3506_defconfig
        ├─ 补默认值 / 解依赖
        └─ 写 .config + .config.old
```

### 6.2 叶子清单

被编译的源文件（仅 4 个 C 文件）：

| 产物 | 源文件 | 编译/链接规则 |
|---|---|---|
| `scripts/basic/fixdep` | `scripts/basic/fixdep.c` | HOSTCC |
| `scripts/kconfig/conf.o` | `scripts/kconfig/conf.c` | HOSTCC `-c` |
| `scripts/kconfig/zconf.tab.o` | `zconf.tab.c`（由 `zconf.tab.c_shipped` 复制） | HOSTCC `-c` |
| `scripts/kconfig/conf` | 上述两个 .o | HOSTLD 链接 |

被读取但不编译的文件：`configs/rk3506_defconfig`、顶层 `Kconfig` 及全部 `source` 链上的 Kconfig 文件。

生成的产物：`.config`、`.config.old`。

典型构建输出：

```
  HOSTCC  scripts/basic/fixdep
  HOSTCC  scripts/kconfig/conf.o
  SHIPPED scripts/kconfig/zconf.tab.c
  SHIPPED scripts/kconfig/zconf.lex.c
  SHIPPED scripts/kconfig/zconf.hash.c
  HOSTCC  scripts/kconfig/zconf.tab.o
  HOSTLD  scripts/kconfig/conf
#
# configuration written to .config
#
```

---

## 7. 规则触发机制

### 7.1 make 的两阶段模型

1. **解析阶段**：make 读取命令行指定的 makefile 文件并逐行解析，将全部规则与变量登记进内部规则库，此阶段不执行命令。
2. **执行阶段**：从目标（goal）出发，查询规则库、构建依赖图、按需执行命令。

### 7.2 模式规则的匹配

`make rk3506_defconfig` 中，目标 `rk3506_defconfig` 在顶层 Makefile 无显式规则，经隐式规则搜索命中模式规则 `%config`（`rk3506_defconfig` 以 `config` 结尾，stem 为 `rk3506_def`）。该规则命令体发起递归 make，子进程以 `scripts/Makefile.build` 为 makefile，目标仍为 `rk3506_defconfig`；子进程规则库中的 `%_defconfig` 命中（stem 为 `rk3506`），前置 `scripts/kconfig/conf` 触发 host 规则链构建。

目标名 `rk3506_defconfig` 在磁盘上不是真实文件，make 判定目标恒过期，规则命令体每次执行，`.config` 每次重新生成——配置命令须幂等可重跑。

### 7.3 include 机制

`scripts/Makefile.build` 第 57~59 行：

```make
kbuild-dir  := $(if $(filter /%,$(src)),$(src),$(srctree)/$(src))
kbuild-file := $(if $(wildcard $(kbuild-dir)/Kbuild),$(kbuild-dir)/Kbuild,$(kbuild-dir)/Makefile)
include $(kbuild-file)
```

- 命令行变量 `obj=scripts/kconfig` 经 `src` 传递，`kbuild-dir` 为 `$(srctree)/scripts/kconfig`；
- `scripts/kconfig/` 下不存在 `Kbuild` 文件（`wildcard` 为空），故 `kbuild-file` 取 `Makefile`；
- `include` 将该文件内容并入当前解析流：其中的 `hostprogs-y`、`conf-objs` 等变量与 `%_defconfig` 规则立即进入共享命名空间与规则库；
- `include` 与递归 make 的区别：前者为同一进程内的解析期包含（规则库合一、变量共享），后者为独立进程（规则库隔离、仅传导出变量）。

---

## 8. 关键文件索引

| 文件 | 作用 |
|---|---|
| `configs/rk3506_defconfig` | RK3506 默认配置（171 行） |
| `Kconfig`（顶层） | 配置树根，`source` 各子系统 Kconfig |
| `arch/arm/Kconfig` | `SYS_CPU` 默认 `"armv7"`（CPU_V7 时） |
| `arch/arm/mach-rockchip/Kconfig` | `ROCKCHIP_RK3506` 定义（第 384 行）、`TPL_LDSCRIPT`/`TPL_TEXT_BASE`/`TPL_MAX_SIZE`（第 404~415 行） |
| `arch/arm/mach-rockchip/rk3506/Kconfig` | `TARGET_EVB_RK3506`、`SYS_SOC="rockchip"` |
| `board/rockchip/evb_rk3506/Kconfig` | `SYS_BOARD`、`SYS_VENDOR`、`SYS_CONFIG_NAME` |
| `include/configs/rk3506_common.h` / `evb_rk3506.h` | SoC/板级配置头文件 |
| `Makefile`（顶层） | `%config`（485 行）、`scripts_basic`（404 行）、`auto.conf` 规则（515 行）、`u-boot` 链接（1290 行）、SPL/TPL 目标（1446/1465 行） |
| `scripts/Makefile.build` | kbuild 降目录构建入口（`include` kbuild-file、host 规则装载）；`builtin-target` 判定（108~110 行）、built-in.o 链接规则（350~361 行）、递归下降（423~425 行） |
| `scripts/Makefile.host` | 宿主程序编译/链接规则（host-cobjs/host-cmulti，103~116 行） |
| `scripts/Makefile.lib` | `_shipped` 复制规则（249~252 行）、DTB 规则、`multi_depend` 依赖注入（198~202 行） |
| `scripts/Makefile.spl` | SPL/TPL 构建（`obj=spl`/`obj=tpl` 时的规则库） |
| `scripts/Makefile.autoconf` | `include/config.h`、`include/autoconf.mk`、符号链接生成 |
| `scripts/kconfig/Makefile` | `%_defconfig`（120 行）、`hostprogs-y`（204 行） |
| `scripts/kconfig/conf.c` | conf 程序主逻辑 |
| `scripts/kconfig/zconf.y` / `zconf.l` | Kconfig 语言文法与词法 |
| `scripts/kconfig/zconf.tab.c_shipped` 等 | 预生成的解析器代码 |
| `dts/Makefile` | `dt.dtb`/`dt-spl.dtb`/`dt-tpl.dtb` 生成（fdtgrep 裁剪） |
| `arch/arm/dts/rk3506-evb.dts` 等 | RK3506 设备树源 |
| `make.sh` | 瑞芯微构建入口（配置、编译、打包调度） |
| `scripts/fit.sh` / `fit-core.sh` | FIT 镜像打包 |
| `arch/arm/mach-rockchip/make_fit_optee.sh` | ITS 生成脚本（`CONFIG_SPL_FIT_GENERATOR`） |

---

## 9. Kconfig 语法参考

### 9.1 语法总则

- 每条语句以换行结束；行首 `#` 为注释。
- 符号名约定全大写（省略 `CONFIG_` 前缀），如 `ROCKCHIP_RK3506`。
- 字符串字面量须用双引号包裹；数值可为十进制或 `0x` 十六进制。
- 缩进无语义作用，仅用于可读性；`help` 文本的缩进除外（见 9.2）。
- 语句可按条件嵌套于 `if ... endif` 与 `menu ... endmenu` 块内。

### 9.2 符号定义

语法模板（属性顺序非强制，通常按此排列）：

```
config <SYMBOL>
	<type> [prompt]
	[default <expr> [if <expr>]]
	[depends on <expr>]
	[select <SYMBOL> [if <expr>]]
	[imply <SYMBOL> [if <expr>]]
	[range <min> <max>]
	[option <name>[="<value>"]]
	[help]
	  <帮助文本；缩进的行均属于帮助文本>
```

类型：

| 类型 | 取值 | 说明 |
|---|---|---|
| `bool` | y / n | 布尔开关 |
| `tristate` | y / m / n | 三态；U-Boot 无模块机制，实际不使用 m，语法保留 |
| `string` | 字符串 | 如 `CONFIG_USB_GADGET_MANUFACTURER` |
| `int` / `hex` | 整数 / 十六进制 | 如 `CONFIG_DEBUG_UART_BASE` |

提示文本（prompt）两种写法：`bool "Enable foo"`（类型后直接跟提示），或单独一行 `prompt "Enable foo"`。**无 prompt 的符号为内部符号**：不可在 menuconfig 中显示与设置，只能被 `select`/`imply`/表达式引用，其值仅由 `default` 决定。典型实例：`board/rockchip/evb_rk3506/Kconfig` 中的 `SYS_BOARD`、`SYS_VENDOR`、`SYS_CONFIG_NAME`。

### 9.3 表达式与条件

- 运算符：`&&`（逻辑与）、`||`（逻辑或）、`!`（逻辑非），支持括号。
- 条件位置：`depends on <expr>`；`default`/`select`/`imply` 后接 `if <expr>`；块语句 `if <expr> ... endif`。
- 多个 `depends on` 合并为逻辑与。
- 符号值优先级（tristate）：y > m > n；bool 仅 y/n。表达式求值时，未满足依赖的符号视为 n。
- 多个 `default` 存在时，取第一个依赖条件满足者；均不满足时符号取 n。

### 9.4 依赖传播规则

- **select（强依赖）**：`A select B` 时，A 被选为 y 则 B 被强制置 y（tristate 为至少 m），覆盖 B 的默认值与用户设置。若 B 的 `depends on` 不满足，构建时产生 "unmet direct dependencies" 警告，B 的值仍被强制。
- **imply（弱依赖）**：`A imply B` 时，A 为 y 使 B 的默认值变为 y，但 B 可被显式关闭，且 B 的依赖必须满足。适用于"默认开启、允许裁剪"的选项（如 RK3506 的 `imply SPL`、`imply TPL`）。
- **depends on（前置条件）**：条件不满足时符号不可见（menuconfig 不显示）且值为 n。

### 9.5 菜单与界面结构

- `menu "<prompt>" ... endmenu`：菜单块。
- `menuconfig <SYMBOL>`：可展开/折叠的菜单项；其后通常跟 `if <SYMBOL>` 块放置子选项。
- `choice ... endchoice`：单选组，成员须为 bool/tristate；组内仅一个成员可被选中；`optional` 允许全不选。
- `comment "<prompt>"`：菜单内注释行。
- `visible if <expr>`：仅控制菜单的显示，不影响选项值。
- `mainmenu "<prompt>"`：仅允许出现在顶层，定义配置界面标题（见顶层 `Kconfig` 第 6 行）。

### 9.6 其他指令

- `source "<path>"`：包含另一个 Kconfig 文件。路径相对于构建根目录（`zconf_fopen` 依次尝试 `./<path>` 与 `$(srctree)/<path>`）；支持 `$(VAR)` 环境变量展开；递归包含会被检测并终止解析。
- `option env="<VAR>"`：符号值取自环境变量（如顶层 `Kconfig` 的 `UBOOTVERSION`）。
- `option defconfig_list`：配合 `CONFIG_BASE_DEFCONFIG` 实现多级 defconfig 继承（make.sh 与 `scripts/kconfig/merge_config.sh` 使用）。
- `option modules`：模块开关（内核使用，U-Boot 不涉及）。
- `range <min> <max>`：int/hex 符号的值域约束，越界值被裁剪。

### 9.7 defconfig 文件语法

`configs/*_defconfig` 与 `.config` 同构，每行一条：

```
CONFIG_<SYMBOL>=y
# CONFIG_<SYMBOL> is not set
CONFIG_<SYMBOL>="string"
CONFIG_<SYMBOL>=0x...
```

未出现在文件中的符号在配置时按 Kconfig 规则补齐（`default`、`select`、`imply`）。`CONFIG_BASE_DEFCONFIG="<name>"` 可声明基础 defconfig 实现多级继承。

### 9.8 本仓库实例对照

`arch/arm/mach-rockchip/Kconfig` 第 384~415 行：

```kconfig
config ROCKCHIP_RK3506
	bool "Support Rockchip RK3506"
	select CPU_V7
	select ROCKCHIP_BROM_HELPER
	select DEBUG_UART_BOARD_INIT
	select ARM_SMCCC
	select GICV2
	imply SUPPORT_SPL
	imply SPL
	imply TPL
	imply TPL_TINY_FRAMEWORK if TPL
	imply BOARD_LATE_INIT
	imply ROCKCHIP_GPIO_V2

if ROCKCHIP_RK3506

config TPL_LDSCRIPT
	default "arch/arm/mach-rockchip/u-boot-tpl.lds"

config TPL_TEXT_BASE
	default 0xfff81000

config TPL_MAX_SIZE
	default 45056

endif
```

其中 `select` 为强制依赖（如 `CPU_V7`，决定 `SYS_CPU="armv7"` 及 `-march=armv7-a` 编译选项），`imply` 为默认开启（如 `SPL`/`TPL`），`if ... endif` 限定 RK3506 专用符号的作用范围。配置完成后执行 `grep -E "CPU_V7|^CONFIG_SPL=|^CONFIG_TPL=|TPL_TINY" .config` 可验证传播结果。

---

## 10. 附录：验证方法

- `make -d rk3506_defconfig`：输出 make 的决策日志，可观察模式规则匹配过程（"Trying pattern rule"、stem 判定）。
- `make -f scripts/Makefile.build obj=scripts/kconfig -p rk3506_defconfig`：`-p` 打印规则库，可检索 `%_defconfig` 等规则。
- 配置完成后核对连锁反应：`grep -E "CPU_V7|^CONFIG_SPL=|^CONFIG_TPL=|TPL_TINY" .config`。
- 编译后核对各阶段产物：`spl/u-boot-spl.map`、`u-boot.map` 中的符号，或对照各目录 Makefile 的 `obj-*` 行。
