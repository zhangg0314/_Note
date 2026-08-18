# Makefile 与 U-Boot 构建知识点整理

> 本文档整理自一次关于 Makefile 语法与 U-Boot（Rockchip RK3506 / TL3506-MiniEVM）构建的学习对话。
> 源码树：`E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\U-Boot\src\u-boot`（U-Boot 2017.09 内核 kbuild 体系，Rockchip 定制版）

---

## 目录

1. [Makefile 中"文件名"的本质：字符串与路径解析时机](#1-makefile-中文件名的本质字符串与路径解析时机)
2. [Makefile 常用函数大全](#2-makefile-常用函数大全)
3. [为什么要有 `$(shell ...)`：make 的两阶段模型](#3-为什么要有-shell-make-的两阶段模型)
4. [深度解析 kbuild 增量构建核心代码：`cmd_files := $(wildcard ...)`](#4-深度解析-kbuild-增量构建核心代码)
5. [实战排错：U-Boot 编译报错 `-mabi=aapcs-linux` 分析与修复](#5-实战排错u-boot-编译报错分析)
6. [附：本板子（RK3506）的正确构建方法](#6-附本板子rk3506的正确构建方法)

---

## 1. Makefile 中"文件名"的本质：字符串与路径解析时机

### 核心语义

GNU Make 的一条核心语义：**Makefile 里的"文件名"只是字符串，路径解析发生在"使用"时而不是"赋值"时。**

### 逐句拆解典型注释

> 一个文件名正常应该是带绝对路径的文件名，缺省绝对路径只有文件名的话，默认都是当前目录下的文件。

- 提到文件（目标、依赖、源文件）可以写绝对路径 `/home/xxx/main.c`，也可以只写文件名 `main.c`。
- 省略路径只写文件名时，**隐含指当前目录**（make 运行时的工作目录 CWD，可用 `$(CURDIR)` 查看）下的文件。
- 这是提醒：裸文件名有"默认路径"约定，该约定在文件系统真正被访问时才生效。

> 当文件名（无论有路径）赋值给变量时，其特性就不是文件了，就是一个字符串；赋值的时候缺省路径，那么该字符串就没有路径，变量就是变量。

- make 的变量**只有字符串这一种类型**，没有"文件对象"。
- 赋值那一刻，make **不会**检查文件是否存在，也**不会**解析/补齐路径。
- 只写 `main.c`，变量里就是没有路径的裸字符串 `main.c`；写了绝对路径，变量里就带着那段路径文本。

### 路径解析发生在"使用"时

| 时机 | 发生了什么 |
|------|-----------|
| 赋值时 | 纯字符串操作，不碰文件系统 |
| 作为依赖/目标时 | make 才去文件系统找文件，相对路径按工作目录（或 `VPATH`/`vpath`）解析 |
| 在 recipe 命令行里 | 传给 shell 执行，由 shell 按当前目录解析相对路径 |

```make
SRCS = main.c          # 只是字符串 "main.c"
main: $(SRCS)
	gcc $(SRCS) -o main   # 这里才真的在当前目录找 main.c
```

### 实际指导意义

1. **路径安全**：想让路径可靠，显式加工：`$(CURDIR)/...`、`$(abspath ...)`、`$(realpath ...)`，或写成 `src/main.c` 这类相对路径，而不是裸文件名。
2. **不要假设**：`OBJS = main.o foo.o` 只是名字，最终在哪生成/查找取决于规则目标和 make 的运行目录。
3. **一句话总结**：**规则里的是文件，变量里的是字符串；裸文件名 = 默认当前目录，这个默认值在赋值时不会自动补进字符串里。**

---

## 2. Makefile 常用函数大全

> 语法：`$(函数 参数...)`，示例尽量取自 U-Boot 顶层 Makefile 的真实用法。

### 2.1 字符串处理类

| 函数 | 作用 | 示例 |
|------|------|------|
| `$(subst from,to,text)` | 把 text 中所有 `from` 替换为 `to` | `$(subst .c,.o,main.c)` → `main.o` |
| `$(patsubst p,r,text)` | 按模式替换（`%` 通配） | `$(patsubst %.c,%.o,main.c)` → `main.o` |
| `$(strip str)` | 去首尾空白并压缩连续空格 | `$(strip "  a   b ")` → `a b` |
| `$(findstring find,in)` | 在 in 里找 find，找到返回 find，否则空 | `$(findstring a,abc)` → `a` |
| `$(filter p...,text)` | 保留匹配模式的单词 | `$(filter %.c,main.c main.o)` → `main.c` |
| `$(filter-out p...,text)` | 去掉匹配模式的单词 | `$(filter-out %.o,main.c main.o)` → `main.c` |
| `$(sort list)` | 按字典序排序并去重 | `$(sort b a c a)` → `a b c` |
| `$(word n,text)` | 取第 n 个单词（从 1 开始） | `$(word 2,a b c)` → `b` |
| `$(wordlist s,e,text)` | 取第 s 到第 e 个单词 | `$(wordlist 2,3,a b c d)` → `b c` |
| `$(words text)` | 统计单词个数 | `$(words a b c)` → `3` |
| `$(firstword text)` | 取第一个单词 | `$(firstword a b c)` → `a` |
| `$(lastword text)` | 取最后一个单词 | `$(lastword a b c)` → `c` |

**快捷写法**：`$(var:%.c=%.o)` ≡ `$(patsubst %.c,%.o,$(var))`；`$(var:.c=.o)` ≡ `$(subst .c,.o,$(var))`。最常用：

```make
OBJS = $(SRCS:.c=.o)        # 或 $(SRCS:%.c=%.o)
```

### 2.2 文件名处理类

| 函数 | 作用 | 示例 |
|------|------|------|
| `$(dir names...)` | 取目录部分（含末尾 `/`） | `$(dir src/main.c)` → `src/` |
| `$(notdir names...)` | 取文件名部分 | `$(notdir src/main.c)` → `main.c` |
| `$(suffix names...)` | 取后缀 | `$(suffix main.c)` → `.c` |
| `$(basename names...)` | 去掉后缀 | `$(basename main.c)` → `main` |
| `$(addprefix p,names...)` | 加前缀 | `$(addprefix obj/,a.o b.o)` → `obj/a.o obj/b.o` |
| `$(addsuffix s,names...)` | 加后缀 | `$(addsuffix .o,a b)` → `a.o b.o` |
| `$(join l1,l2)` | 两个列表按位置拼接 | `$(join a b,1 2)` → `a1 b2` |
| `$(wildcard pattern)` | 展开通配符，返回真实存在的文件 | 见下方 U-Boot 示例 |
| `$(realpath names...)` | 绝对路径（**要求文件存在**，否则空） | `$(realpath ./Makefile)` |
| `$(abspath names...)` | 绝对路径（**不要求存在**，只做规范化） | `$(abspath ./nonexist)` |

**U-Boot 真实例子**（顶层 Makefile）：

```make
# 判断配置文件是否存在
ifneq ($(wildcard include/config/auto.conf),)
    ...
endif

# 收集 .cmd 依赖文件：对每个 target 取目录 + 取文件名
cmd_files := $(wildcard .*.cmd $(foreach f,$(targets),$(dir $(f)).$(notdir $(f)).cmd))

# wildcard 与 if 配合：文件存在返回 y，否则 n
HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(srctree)/board/$(VENDOR)/common/Makefile),y,n)
```

### 2.3 条件类函数

| 函数 | 作用 |
|------|------|
| `$(if cond,then[,else])` | cond 非空则展开 then，否则展开 else（else 可省略） |
| `$(or a,b,...)` | 从左到右返回第一个非空值 |
| `$(and a,b,...)` | 全部非空才返回最后一个值，否则空 |

注意：这些是**函数**（表达式内求值），与 `ifeq`/`ifdef` 等**条件指令**（控制整个 Makefile 分支）不同。

```make
HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(srctree)/board/$(VENDOR)/common/Makefile),y,n)
CFLAGS = $(if $(DEBUG),-g,-O2)
```

### 2.4 循环函数

`$(foreach var,list,text)` —— 把 list 里的每个单词依次赋给 `var`，展开 `text`，结果按空格拼接：

```make
# U-Boot 中：遍历所有子目录，只保留带 Makefile 的目录
clean-dirs := $(foreach f,$(u-boot-alldirs),$(if $(wildcard $(srctree)/$f/Makefile),$f))
```

注意：循环体里要修改别的变量需配合 `$(eval)` 才能持久生效。

### 2.5 执行 shell 命令

`$(shell command)` —— 执行 shell 命令并把**标准输出**作为结果（末尾换行被去掉）：

```make
ARCH := $(shell uname -m)
DATE := $(shell date +%Y%m%d)
SRCS := $(shell find src -name "*.c")
```

⚠️ 注意：
1. 每次 `$(shell)` 都起一个子进程，**别在 `$(foreach)` 大循环里滥用**。
2. 命令输出为空时变量是空字符串，`ifdef` 判断会失效，常配合 `$(strip)` 或 `$(if)` 使用。

### 2.6 控制类函数（调试/报错）

| 函数 | 作用 |
|------|------|
| `$(info text)` | 打印信息，不报错，**总是求值** |
| `$(warning text)` | 打印警告，make 继续执行 |
| `$(error text)` | 打印错误并**立即终止** make |

```make
ifeq ($(CONFIG_CPU),)
$(error CONFIG_CPU is not set! Please configure first)
endif
```

调试技巧：`$(info VAR=$(VAR))` 或 `$(warning ...)` 是排查变量展开问题最常用的手段。

### 2.7 其他常用函数

| 函数 | 作用 | 说明 |
|------|------|------|
| `$(call name,arg1,arg2,...)` | 调用自定义"函数"（带参变量） | `define name` ... `endef`，内部用 `$(1)` `$(2)` 引用参数 |
| `$(eval text)` | 把 text 当作 Makefile 内容动态展开解析 | 配合 `$(foreach)` 动态生成规则 |
| `$(value var)` | 取变量的**原始未展开**值 | 对递归变量有用 |
| `$(origin var)` | 返回变量来源（`command line`/`environment`/`file`/`undefined` 等） | U-Boot 用 `ifeq ("$(origin V)", "command line")` 判断 V 是否来自命令行 |
| `$(flavor var)` | 返回 `recursive` 或 `simple` | 判断变量是 `=` 还是 `:=` 定义的 |
| `$(file op filename,text)` | 读写文件（`>` 写、`>>` 追加、`<` 读） | 较新版本才有 |
| `$(intcmp ...)`、`$(let ...)` | 整数比较、局部变量绑定 | GNU make 4.4+ 才有，老环境慎用 |

### 2.8 记忆要点

1. **参数个数差异**：有的参数用逗号分隔多个（如 `subst from,to,text`），有的参数是单词列表（如 `$(sort ...)`），有的参数本身就是表达式（如 `$(if ...)` 里可以嵌套）。
2. **全部是"展开时求值"**：make 先展开所有函数，得到纯字符串，然后才用于规则/命令。
3. **最常用的就这几个**：`wildcard`、`patsubst`（或 `:%.c=%.o` 简写）、`foreach`、`if`、`filter`、`notdir`、`dir`、`shell`、`error/warning/info`。写板级/驱动 Makefile 基本离不开 `obj-y += xxx.o` + 这些函数的组合。
4. 查看完整手册：`info make` 或 GNU Make Manual 的 **Functions** 章节。

---

## 3. 为什么要有 `$(shell ...)`：make 的两阶段模型

### 先纠正一个概念

**Makefile 不是 shell 脚本。** make 是独立程序，自己解析 Makefile（变量、规则、函数、条件），**只有规则下面的命令缩进行（recipe）**才会在"执行阶段"被逐行交给 `/bin/sh` 执行。

### 关键：make 的两个阶段

| 阶段 | 做什么 | 有 shell 吗 |
|------|--------|-------------|
| **读取阶段**（解析 Makefile） | 展开变量、求值函数、判断 `ifeq`、收集规则 | ❌ 没有，纯文本处理 |
| **执行阶段**（remake） | 找出过期目标，逐行执行其 recipe | ✅ 有，每行开一个 shell 进程 |

`$(shell ...)` 的作用：让 make 在**读取/展开阶段**主动调用一次 shell，把命令的**标准输出抓回来当字符串用**。

> recipe 里的命令 = "执行阶段"的 shell；`$(shell ...)` = "读取阶段"临时借用的 shell。

### U-Boot 真实例子

```make
# 赋值时就要拿到主机架构，这不是任何规则的 recipe
HOSTARCH := $(shell uname -m | sed -e s/i.86/x86/ ...)

# 问编译器它的内部 include 目录在哪，结果拼进编译选项
NOSTDINC_FLAGS += -nostdinc -isystem $(shell $(CC) -print-file-name=include)

# ifeq 条件需要命令的输出来决定分支
ifneq ($(shell $(CROSS_COMPILE)ld.bfd -v 2> /dev/null),)
    ...
endif
```

这些命令的结果**必须在 make 解析 Makefile 的当下**变成变量值或条件判断依据。等到执行阶段就来不及了。

### 同一个动作的两种写法对比

```make
DATE := $(shell date)      # 读取阶段：make 读到这一行时立即执行
all:
	date                    # 执行阶段：只有当 all 被选中且过期时，shell 才执行
```

### 常见坑

1. **recipe 每行是独立的 shell 进程**（默认，`.ONESHELL` 除外）：`cd src` 只对当前行有效，下一行 `pwd` 又回到原处；`$(shell ...)` 内部可以用 `&&` 串命令。
2. **输出去向不同**：recipe 输出直接到终端；`$(shell ...)` 的 **stdout 被吞掉变成字符串**，stderr 仍直接漏到终端。
3. **失败处理不同**：recipe 命令非零 → make 报错终止（除非行首加 `-`）；`$(shell ...)` 退出码**默认被忽略**（新版可用 `$(.SHELLSTATUS)` 查看），所以常写 `2> /dev/null` 自己吞错误。
4. **展开次数不同**：`:=` 赋值只执行一次；`=` 递归赋值每次引用都重新执行。
5. **性能**：`$(shell ...)` 每次 fork 子进程，别在大循环里滥用。

### 一句话总结

> Makefile 的"执行阶段"才由 shell 跑命令，而"读取/展开阶段"没有 shell。想在**解析期间**（定变量、判条件、造目标名）拿到命令结果，就用 `$(shell ...)`；想**构建某个目标时**跑命令，就写进 recipe。

---

## 4. 深度解析 kbuild 增量构建核心代码

```make
cmd_files := $(wildcard .*.cmd $(foreach f,$(targets),$(dir $(f)).$(notdir $(f)).cmd))
```

> 出处：U-Boot 顶层 `Makefile` 第 1743 行，注释 "read all saved command lines"。`scripts/Makefile.build` 第 440 行有同款（无 `.*.cmd` 项）。

### 这行代码在干什么

收集上次构建保存的所有 `.cmd` 文件，随后 `include $(cmd_files)`（顶层 Makefile 1745-1748 行）把上次每条编译命令加载回 make 变量，供 `if_changed` 判断"命令变没变、要不要重编"。

**背景知识——`.cmd` 文件**：
- kbuild 每编译出一个目标（如 `common/main.o`），会把**实际执行的命令**和**依赖列表**写进隐藏文件 `common/.main.o.cmd`。
- 生成逻辑在 `scripts/Kbuild.include`：
  - 第 14 行：`dot-target = $(dir $@).$(notdir $@)` → `common/.main.o`
  - 第 261 行：`printf 'cmd_$@ := ...' > $(dot-target).cmd`
  - 第 267-269 行：`fixdep` 再把 `.d` 依赖文件合并进去 → `.cmd` 里同时有 `cmd_... := 命令` 和 `deps_... := 依赖清单`

### 逐块拆解

| 部分 | 含义 |
|------|------|
| `:=` | 立即赋值，只算一次 |
| `$(foreach f,$(targets), ...)` | 遍历目标列表。`targets` 在 `scripts/Makefile.build` 里逐步累积（318-319、361、387、417 行）：`targets += $(real-objs-y) $(real-objs-m) $(lib-y) $(extra-y) $(MAKECMDGOALS) $(always) $(builtin-target) $(lib-target) $(multi-used-y) $(multi-used-m)` |
| `$(dir $(f)).$(notdir $(f)).cmd` | 把"目标路径"翻译成".cmd 文件路径"（见下表） |
| `.*.cmd` | 当前目录兜底：顶层 Makefile 的 `$(targets)` 是空的（顶层无 `targets +=`），只能靠它把顶层 `.u-boot.bin.cmd` 这类捞进来；子目录版 `Makefile.build` 靠 `$(dir f)` 返回 `./` 即可，无需此项 |
| `$(wildcard ...)` | 只保留磁盘上真实存在的文件，避免 `include` 不存在的文件报错 |

**`$(dir).$(notdir)` 翻译表**（关键：点号必须在 basename 前，即 `dir/.name.cmd` 而非 `dir/name.cmd`）：

| 目标 f | `$(dir f)` | `$(notdir f)` | 拼接结果 |
|--------|-----------|---------------|----------|
| `common/main.o` | `common/` | `main.o` | `common/.main.o.cmd` |
| `drivers/serial/ns16550.o` | `drivers/serial/` | `ns16550.o` | `drivers/serial/.ns16550.o.cmd` |
| `u-boot.bin` | `./` | `u-boot.bin` | `./.u-boot.bin.cmd` |

> 为什么不能直接写 `$(f).cmd`？那样得到 `common/main.o.cmd`，少了点号。.cmd 命名约定是"点号 + 原名 + .cmd"（`dir/.name.cmd`），点号使它成为隐藏文件。

### 完整演算示例

假设子目录 `common/` 上次构建留下 `common/main.o`、`common/console.o`、`common/built-in.o` 及对应 `.cmd`，`targets`（排序前含重复）= `common/console.o common/main.o common/built-in.o common/main.o`：

1. 第 1742 行先处理：`targets := $(wildcard $(sort $(targets)))` → 排序+去重 → 过滤不存在的 → `common/built-in.o common/console.o common/main.o`
2. `$(foreach ...)` 逐项翻译 → `common/.built-in.o.cmd common/.console.o.cmd common/.main.o.cmd`
3. 拼上 `.*.cmd`（顶层才有内容），外层 `$(wildcard ...)` 过滤
4. 最终 `cmd_files = common/.built-in.o.cmd common/.console.o.cmd common/.main.o.cmd`
5. 第 1745-1748 行：

```make
ifneq ($(cmd_files),)
  $(cmd_files): ;                # 声明这些 .cmd 是"空 recipe 目标"，别去更新它们
  include $(cmd_files)           # 把上次的命令/依赖读进 make
endif
```

`include` 后 make 得到 `cmd_common/main.o := gcc ...`、`deps_common/main.o := ...`，`if_changed` 拿它们与本次命令比对，命令变了就重编。

### 关于 wildcard 参数的一个重要澄清

**`$(wildcard ...)` 的参数就是 `.*.cmd $(foreach ...)` 这一整串。** make 函数参数只按逗号分隔，不按空格；foreach 内部的逗号在嵌套括号里，不算 wildcard 的参数分隔符。展开后是一串空格分隔的词，wildcard 把**每个词**当作一个 pattern 去匹配磁盘：

| 输入词 | 类型 | wildcard 的行为 |
|--------|------|-----------------|
| `.*.cmd` | 通配符模式 | 展开为所有匹配的文件（数量不定） |
| `common/.main.o.cmd` | 具体文件名 | 存在 → 保留；不存在 → 丢掉 |

展开顺序：由内向外 —— 先展开 `$(foreach)`/`$(dir)`/`$(notdir)`，wildcard 最后拿到完全展开的字符串。

### 流程图小结

```
targets（目标名单）
   │  $(sort) 排序去重
   │  $(wildcard) 只留存在的目标
   ▼
foreach：每个目标 f ──► $(dir f) + "." + $(notdir f) + ".cmd"     （dir/.name.cmd）
   │
   ├─ 顶层再补上 .*.cmd 兜底（顶层 targets 为空）
   ▼
$(wildcard ...) 只保留磁盘上真实存在的 .cmd
   ▼
cmd_files ──► include ──► 加载上次命令 cmd_xxx 和依赖 deps_xxx ──► if_changed 判断增量
```

**一句话：把"管理的目标列表"翻译成"各自的 .cmd 记录文件路径"，过滤掉不存在的，再 include 回来，让 make 记住上次是怎么编译的，实现"命令变了才重编"的增量构建。**

---

## 5. 实战排错：U-Boot 编译报错分析

### 报错现场

```bash
$ make arch=ARM u-boot
scripts/kconfig/conf  --silentoldconfig Kconfig
  CHK     include/config.h
  CFG     u-boot.cfg
gcc: error: unrecognized argument in option ‘-mabi=aapcs-linux’
gcc: note: valid arguments to ‘-mabi=’ are: ms sysv
make[1]: *** [scripts/Makefile.autoconf:79: u-boot.cfg] Error 1
make: *** No rule to make target 'include/config/auto.conf', needed by 'include/config/uboot.release'.  Stop.
```

### 两个报错分别是什么

**报错 1：`gcc: error: unrecognized argument in option '-mabi=aapcs-linux'`**

- 这个 `gcc` 是**宿主机 x86-64 的 gcc**（`-mabi` 只接受 `ms`/`sysv` 是 x86-64 特有提示），说明编译 ARM 代码时用了宿主机编译器。
- `u-boot.cfg`（`scripts/Makefile.autoconf:79`）拿 `$(CPP) $(c_flags) -dM include/common.h` 预处理提取所有 `CONFIG_` 宏；`c_flags` 带上了 ARM 专属 `-mabi=aapcs-linux`（来自 `arch/arm/cpu/armv7/config.mk` 第 16 行 `PLATFORM_CPPFLAGS += -mabi=aapcs-linux -funwind-tables`）。
- `CPP = $(CROSS_COMPILE)gcc -E`（Makefile.autoconf 第 30-31 行）——**CROSS_COMPILE 是空的，所以用的是宿主机 gcc**。

**报错 2：`make: *** No rule to make target 'include/config/auto.conf', needed by 'include/config/uboot.release'. Stop.`**

- **连锁反应**：顶层 Makefile 第 515-525 行生成 `include/config/auto.conf` 的 recipe 中写明（517-520 行注释）：`make -f scripts/Makefile.autoconf` 一旦失败，就**故意 `rm -f include/config/auto.conf`**，让下次构建重新跑配置。第一次 make 失败后 auto.conf 被删，后续 `include/config/uboot.release`（1328 行）需要它时树已不一致 → 报 "No rule"。**根因解决后此错误自然消失，无需单独修。**

### 根本原因（两个错误叠加）

**1. make 变量名大小写错误（make 变量区分大小写）**

- `make arch=ARM u-boot`：设置的是小写变量 `arch`，Makefile 用的是大写 `ARCH` → **参数完全没生效**（值 `ARM` 大写也错，源码目录是 `arch/arm/`）。
- `make Arch=arm u-boot`：`Arch` 同样是无关变量。
- 所以 `ARCH` 实际来自 `config.mk`：`ARCH := $(CONFIG_SYS_ARCH)`（源码根 `config.mk` 第 25 行），由之前配置的板子决定 → 本板是 ARM，于是 `arch/arm/config.mk`、`arch/arm/cpu/armv7/config.mk` 被 `sinclude`，`-mabi=aapcs-linux` 被加进编译参数。

**2. CROSS_COMPILE 没设（致命）**

- 顶层 Makefile 第 253 行：`CROSS_COMPILE ?=`（默认为空）。
- `CC = $(CROSS_COMPILE)gcc` → `CC = gcc` = 宿主机 x86-64 gcc → 拒绝 ARM 的 `-mabi=aapcs-linux`。

### 排查路径参考（源码证据链）

- `scripts/Makefile.autoconf:27-33`：`CC = $(CROSS_COMPILE)gcc`；`CPP = $(CC) -E`；`include config.mk`
- `scripts/Makefile.autoconf:69-79`：`cmd_u_boot_cfg = $(CPP) $(c_flags) ... -dM include/common.h > $@`；`u-boot.cfg: include/config.h FORCE`
- `arch/arm/cpu/armv7/config.mk:16`：`PLATFORM_CPPFLAGS += -mabi=aapcs-linux -funwind-tables`
- 根 `config.mk:25`：`ARCH := $(CONFIG_SYS_ARCH:"%"=%)`；`46-47`：`sinclude arch/$(ARCH)/config.mk`、`sinclude $(CPUDIR)/config.mk`
- 顶层 `Makefile:515-528`：auto.conf 生成 recipe 及其失败删除逻辑
- 顶层 `Makefile:1328`：`include/config/uboot.release: include/config/auto.conf FORCE`

---

## 6. 附：本板子（RK3506）的正确构建方法

> 本源码树是 Rockchip 定制版 U-Boot。板子：**RK3506（三核 Cortex-A7，32 位 ARMv7）**，TL3506-MiniEVM 即 RK3506 平台的厂商命名。基础 defconfig：`configs/rk3506_defconfig`（另有 `rk3506-amp.config`、`rk3506_tb.config`、`rk3506b.config` 变体片段）。

### 方式一：厂商脚本（推荐）

源码根目录 `make.sh` 是 Rockchip 官方构建脚本，自动选 defconfig、自动带交叉编译器：

```bash
./make.sh rk3506
```

等价于（make.sh 第 252、804 行）：

```bash
make rk3506_defconfig
make CROSS_COMPILE=../prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- all
```

### 方式二：手动三步

```bash
make rk3506_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- all
```

### 注意事项

1. **工具链必须存在**。`make.sh` 第 15 行写死 `../prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/...`（源码目录**上一级**的 `prebuilts` 目录），第 299 行会 `which` 检查。要么按此相对路径放工具链，要么用自己装的（`sudo apt install gcc-arm-linux-gnueabihf`，前缀 `arm-linux-gnueabihf-`）。
2. **`make.sh` 还需要 `../rkbin` 仓库**（第 14、105-111 行，没有则报 "ERROR: No ../rkbin repository"）。只编译 u-boot 二进制可用方式二手动 make；打包 `uboot.img`/`trust.img`/loader 镜像必须有 rkbin。
3. **失败后先清理再重来**：`make distclean`（或 `rm -rf include/config/auto.conf*`）→ `make rk3506_defconfig` → `make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- all`。
4. 若厂商文档要求合并某个配置片段（如 `rk3506_tb.config`），按 SDK 高层构建脚本处理，基础仍是 `rk3506_defconfig`。

### 一句话总结

> 报错不是 make 语法问题，而是**用宿主机 gcc 编 ARM 代码**：`arch`/`Arch` 大小写不对 → `ARCH` 实际来自已配置的板级信息（arm）→ `CROSS_COMPILE` 为空 → `CC=gcc` 是 x86 编译器 → 不认识 `-mabi=aapcs-linux`。补上交叉编译器前缀并用对 `ARCH=arm`，或直接 `./make.sh rk3506` 即可；第二个 "No rule to make target" 只是 auto.conf 被按设计删除后的连带报错。
