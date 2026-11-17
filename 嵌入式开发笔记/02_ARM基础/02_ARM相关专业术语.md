.pushsection .text.lowlevel_init, "ax"是啥

```
■ Conversation interrupted - use /feedback if something went wrong
```

.pushsection .text.lowlevel_init, "ax"是啥

这是 GNU 汇编器的指令，意思是：**暂时切换到名为 `.text.lowlevel_init` 的代码段，后面的 `lowlevel_init` 函数放在这个段里。**

```
.pushsection .text.lowlevel_init, "ax"
```

- `.text.lowlevel_init`：输入段的名字；开头的 `.` 只是名字的一部分，不是链接脚本里的“当前位置”。
- `"a"`：这个段要装入运行时内存。
- `"x"`：这个段包含可执行指令。
- `.pushsection`：切换段的同时，记住之前所在的段；后面的 `.popsection` 会切回去。这里的“push”**不是往 CPU 的栈里压数据**。

结合你之前的链接脚本，`*(.text*)` 能匹配 `.text.lowlevel_init`，所以链接时通常会把它归入最终的 `.text` 代码区域。

```
  Worked for 11s • 13:47
```

\- .pushsection：切换段的同时，记住之前所在的段；后面的 .popsection 会切回去啥意思

意思是：**汇编器正在把后续代码写入某个段；`.pushsection` 先记住原来的段，再换到新段；`.popsection` 回到原来的段。**

看这个例子：

```
.section .text
func_a:
    bx lr                 @ 放进 .text

.pushsection .text.lowlevel_init, "ax"
lowlevel_init:
    bx lr                 @ 放进 .text.lowlevel_init
.popsection

func_b:
    bx lr                 @ 又放进 .text
```

所以 `.popsection` 的作用是防止后面定义的 `func_b` 也被放进 `.text.lowlevel_init`。

这是**汇编时整理代码所属段**的操作，不是 CPU 运行时切换代码段，也不会操作 CPU 的栈。

```
  Worked for 12s • 13:48
```

为啥要这样搞？搞这么多段

主要是为了让**链接器能单独处理这些函数**，不是 CPU 运行时真的要管理很多“代码区”。

例如把 `lowlevel_init` 放进 `.text.lowlevel_init`，链接器就可以：

- 按链接脚本指定它的位置；
- 如果启用了按段删除未使用代码，可以单独保留或丢弃它；
- 通过 `*(.text*)` 把它和其他代码段一起合并到最终的 `.text` 区域。

所以在 `.o` 文件里可以有 `.text.s_init`、`.text.lowlevel_init` 等多个**输入段**；最终镜像里，它们通常仍排列在同一个代码区域。`.pushsection` / `.popsection` 则是为了只把这一个函数放进指定段，之后继续按原来的段编译。

对你贴的链接脚本来说，`*(.text*)` 会接收这些段；**仅凭这两行还不能说 `lowlevel_init` 被放到了某个特殊地址**。

```
  Worked for 14s • 13:49
```

没看懂

就看你这份代码的**实际结果**：

```
汇编时：
s_init         → .text.s_init
lowlevel_init  → .text.lowlevel_init
其他代码        → 原来的段

链接时：
*(.text*)     → 把上面这些 .text 开头的段都放进最终的 .text
```

所以它先给 `lowlevel_init` 单独起了一个段名，方便链接器以后**单独找到、安排或删除这个函数**。`.pushsection` 是开始把代码放进这个段；`.popsection` 是这个函数写完后，恢复原来的段，避免后续代码也被归进去。

**在你目前贴出的链接脚本里，`*(.text*)` 把它们一起收进去，并没有特别安排 `lowlevel_init`。**因此就这份脚本而言，分段主要是给链接器留下单独处理的能力；CPU 执行函数时不关心这些段名。

```
  Worked for 12s • 13:51
```

\#ifdef CONFIG_SPL_DM
 mov	r9, #0
 \#else
 /*
  \* Set up global data for boards that still need it. This will be
  \* removed soon.
  */
 \#ifdef CONFIG_SPL_BUILD
 ldr	r9, =gdata
 \#else
 sub	sp, sp, #GD_SIZE
 bic	sp, sp, #7
 mov	r9, sp
 \#endif
 \#endif这是？

这段是在**给 `r9` 设置初始值**。在这份 ARM U-Boot 代码中，`r9` 用来保存全局数据结构 `gd` 的地址。它按编译配置走三选一：

| 编译条件                                            | 执行结果                                         |
| --------------------------------------------------- | ------------------------------------------------ |
| 定义了 `CONFIG_SPL_DM`                              | `r9 = 0`，暂时不提供 `gd` 地址                   |
| 未定义 `CONFIG_SPL_DM`，但定义了 `CONFIG_SPL_BUILD` | `r9 = &gdata`，指向已有的 `gdata` 符号           |
| 两者都未定义                                        | 从当前栈顶留出 `GD_SIZE` 空间，`r9` 指向这块空间 |

最后一种相当于：

```
sub sp, sp, #GD_SIZE   @ 栈顶往低地址移动，留出 gd 的空间
bic sp, sp, #7         @ 重新按 8 字节对齐
mov r9, sp             @ r9 指向留出的空间
```

注意 `ldr r9, =gdata` 取的是 **`gdata` 的地址**，不是读取它的内容；`mov r9, #0` 清零的是**指针 `r9`**，也不是把 `gd` 结构清零。这段只准备地址，尚未初始化 `gd` 的各个字段。

```
  Worked for 24s • 13:56
```

CONFIG_SPL_DM是啥，为啥定义了就不用gd地址

`CONFIG_SPL_DM` 表示 **在 SPL 中启用 U-Boot 的 Driver Model（设备驱动模型）**，用于按设备、驱动和设备类别管理硬件。

关键是：**定义了它，不代表 SPL 不需要 `gd`。**这段代码只是处于非常早的 `lowlevel_init` 阶段，此时还没有把 `gd` 正式建好，所以先执行：

```
mov r9, #0    @ 此刻没有可用的 gd 地址
```

随后进入你之前贴过的 `crt0.S` 中的 `_main`，才会分配并初始化 `gd`，再把地址放进 `r9`。之后的 SPL 代码和 Driver Model 仍然会使用 `gd`。

那为什么未启用 `CONFIG_SPL_DM` 时反而提前设置 `r9`？这是为了兼容一些**旧的板级早期初始化代码**：它们可能在 `s_init` 阶段就想用 `gd`，所以提前给它准备一个地址。代码注释说“for boards that still need it”和“will be removed soon”，指的正是这种旧用法。对使用 SPL Driver Model 的路径，早期 `s_init` 应当不依赖 `gd`。

```
  Worked for 20s • 14:05
• Unrecognized command '/*'. Type "/" for a list of supported commands.
```

：/*
 \*  crt0 - C-runtime startup Code for ARM U-Boot
 *
 \*  Copyright (c) 2012  Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 \* SPDX-License-Identifier:	GPL-2.0+
 */

\#include <config.h>
 \#include <asm-offsets.h>
 \#include <linux/linkage.h>
 \#ifdef CONFIG_CPU_V7M
 \#include <asm/armv7m.h>
 \#endif

/*
 \* This file handles the target-independent stages of the U-Boot
 \* start-up where a C runtime environment is needed. Its entry point
 \* is _main and is branched into from the target's start.S file.
 *
 \* _main execution sequence is:
 *
 \* 1. Set up initial environment for calling board_init_f().
 \*    This environment only provides a stack and a place to store
 \*    the GD ('global data') structure, both located in some readily
 \*    available RAM (SRAM, locked cache...). In this context, VARIABLE
 \*    global data, initialized or not (BSS), are UNAVAILABLE; only
 \*    CONSTANT initialized data are available. GD should be zeroed
 \*    before board_init_f() is called.
 *
 \* 2. Call board_init_f(). This function prepares the hardware for
 \*    execution from system RAM (DRAM, DDR...) As system RAM may not
 \*    be available yet, , board_init_f() must use the current GD to
 \*    store any data which must be passed on to later stages. These
 \*    data include the relocation destination, the future stack, and
 \*    the future GD location.
 *
 \* 3. Set up intermediate environment where the stack and GD are the
 \*    ones allocated by board_init_f() in system RAM, but BSS and
 \*    initialized non-const data are still not available.
 *
 \* 4a.For U-Boot proper (not SPL), call relocate_code(). This function
 \*    relocates U-Boot from its current location into the relocation
 \*    destination computed by board_init_f().
 *
 \* 4b.For SPL, board_init_f() just returns (to crt0). There is no
 \*    code relocation in SPL.
 *
 \* 5. Set up final environment for calling board_init_r(). This
 \*    environment has BSS (initialized to 0), initialized non-const
 \*    data (initialized to their intended value), and stack in system
 \*    RAM (for SPL moving the stack and GD into RAM is optional - see
 \*    CONFIG_SPL_STACK_R). GD has retained values set by board_init_f().
 *
 \* 6. For U-Boot proper (not SPL), some CPUs have some work left to do
 \*    at this point regarding memory, so call c_runtime_cpu_setup.
 *
 \* 7. Branch to board_init_r().
 *
 \* For more information see 'Board Initialisation Flow in README.
 */

/*
 \* entry point of crt0 sequence
 */

ENTRY(_main)

/*
 \* Set up initial C runtime environment and call board_init_f(0).
 */

\#if defined(CONFIG_TPL_BUILD) && defined(CONFIG_TPL_NEEDS_SEPARATE_STACK)
 ldr	r0, =(CONFIG_TPL_STACK)
 \#elif defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
 ldr	r0, =(CONFIG_SPL_STACK)
 \#else
 ldr	r0, =(CONFIG_SYS_INIT_SP_ADDR)
 \#endif
 bic	r0, r0, #7	/* 8-byte alignment for ABI compliance */
 mov	sp, r0
 bl	board_init_f_alloc_reserve
 mov	sp, r0
 /* set up gd here, outside any C code */
 mov	r9, r0
 bl	board_init_f_init_reserve
 bl	board_init_f_boot_flags

​	bl	board_init_f

\#if ! defined(CONFIG_SPL_BUILD)

/*
 \* Set up intermediate environment (new sp and gd) and call
 \* relocate_code(addr_moni). Trick here is that we'll return
 \* 'here' but relocated.
 */

​	ldr	r0, [r9, #GD_START_ADDR_SP]	/* sp = gd->start_addr_sp */
 bic	r0, r0, #7	/* 8-byte alignment for ABI compliance */
 mov	sp, r0
 ldr	r9, [r9, #GD_BD]		/* r9 = gd->bd */
 sub	r9, r9, #GD_SIZE		/* new GD is below bd */

\#ifndef CONFIG_SKIP_RELOCATE_UBOOT
 adr	lr, here
 ldr	r0, [r9, #GD_RELOC_OFF]		/* r0 = gd->reloc_off */
 add	lr, lr, r0
 \#if defined(CONFIG_CPU_V7M)
 orr	lr, #1				/* As required by Thumb-only */
 \#endif
 ldr	r0, [r9, #GD_RELOCADDR]		/* r0 = gd->relocaddr */
 b	relocate_code
 \#endif
 here:

/*
 \* The "relocate_vectors" is in section: .text.relocate_vectors, if we don't
 \* compile "bl	relocate_vectors", there seems something wrong about sections
 \* layout(U-Boot code section is quite large, I don't find root cause now),
 \* so let's just skip it.
 */
 \#ifdef CONFIG_SKIP_RELOCATE_UBOOT
 b	c_runtime
 \#endif

/*
 \* now relocate vectors
 */
 bl	relocate_vectors

c_runtime:
 /* Set up final (full) environment */

​	bl	c_runtime_cpu_setup	/* we still call old routine here */
 \#endif
 \#if !defined(CONFIG_SPL_BUILD) || (defined(CONFIG_SPL_FRAMEWORK) && !defined(CONFIG_TPL_BUILD))
 \# ifdef CONFIG_SPL_BUILD
 /* Use a DRAM stack for the rest of SPL, if requested */
 bl	spl_relocate_stack_gd
 cmp	r0, #0
 movne	sp, r0
 movne	r9, r0
 \# endif
 ldr	r0, =__bss_start	/* this is auto-relocated! */

\#ifdef CONFIG_USE_ARCH_MEMSET
 ldr	r3, =__bss_end		/* this is auto-relocated! */
 mov	r1, #0x00000000		/* prepare zero to clear BSS */

​	subs	r2, r3, r0		/* r2 = memset len */
 bl	memset
 \#else
 ldr	r1, =__bss_end		/* this is auto-relocated! */
 mov	r2, #0x00000000		/* prepare zero to clear BSS */

clbss_l:cmp	r0, r1			/* while not at end of BSS */
 \#if defined(CONFIG_CPU_V7M)
 itt	lo
 \#endif
 strlo	r2, [r0]		/* clear 32-bit BSS word */
 addlo	r0, r0, #4		/* move to next */
 blo	clbss_l
 \#endif

\#if ! defined(CONFIG_SPL_BUILD)
 bl coloured_LED_init
 bl red_led_on
 \#endif
 /* call board_init_r(gd_t *id, ulong dest_addr) */
 mov     r0, r9                  /* gd_t */
 ldr	r1, [r9, #GD_RELOCADDR]	/* dest_addr */
 /* call board_init_r */
 \#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
 ldr	lr, =board_init_r	/* this is auto-relocated! */
 bx	lr
 \#else
 ldr	pc, =board_init_r	/* this is auto-relocated! */
 \#endif
 /* we should not return here. */
 \#endif

ENDPROC(_main)

这段 `crt0.S` 正好回答上一问：**`lowlevel_init` 里把 `r9` 设为 0，只是暂时没有 `gd`；进入 `_main` 后就正式创建它。**

关键几行是：

```
mov sp, r0
bl  board_init_f_alloc_reserve  @ 从栈所在的内存留出空间
mov sp, r0
mov r9, r0                    @ r9 现在指向 gd
bl  board_init_f_init_reserve  @ 初始化预留区域
bl  board_init_f              @ 此时 C 代码可以使用 gd
```

把你贴过的两份代码接起来，就是：

```
start.S
  → cpu_init_crit
  → lowlevel_init：临时栈；CONFIG_SPL_DM 时 r9 暂设为 0
  → _main：重新设置栈，分配并初始化 gd，把地址放入 r9
  → board_init_f
```

后半段分为两条路：

- **普通 U-Boot**：`board_init_f` 计算新地址和栈位置 → `relocate_code` 搬移 U-Boot → 清 BSS → `board_init_r`。
- **SPL**：跳过这段 U-Boot 代码搬移；按配置可能调整栈和 `gd` → 清 BSS → `board_init_r`。

还有一个容易忽略的时序：**调用 `board_init_f` 时 BSS 尚未清零**，所以这时不能依赖普通的全局变量；代码先在可用内存中准备 `gd`，就是为了让早期初始化有地方保存数据。至于你这块板子的 DDR 是否能在 SPL 的 `board_init_f` 中首次初始化，仍取决于 SPL 实际运行地址是否已是 DDR。

```
  Worked for 25s • 14:08
```

crt0 - C-runtime startup Code for ARM U-Boot是啥

`crt0 - C-runtime startup Code for ARM U-Boot` 的意思是：**ARM 版 U-Boot 在正式运行 C 代码前使用的启动代码**。文件名通常是 `crt0.S`，内容是汇编。

CPU 刚进入 U-Boot 时，不能直接像普通程序那样使用 C 函数：栈、全局数据、BSS 等运行条件还没有准备齐。你贴的 `crt0.S` 就负责按顺序做这些事：

```
start.S 完成更早的 CPU 启动工作
    ↓
crt0.S 的 _main：设置栈、建立 gd
    ↓
调用 C 函数 board_init_f()
    ↓
普通 U-Boot 搬移代码；然后清零 BSS
    ↓
进入 board_init_r()
```

所以这里的 **“C runtime”是“运行 C 代码所需的环境”**，不是说这个文件本身用 C 写成，也不是在启动 Linux。`_main` 是 U-Boot 的汇编入口名，不是普通 C 程序里的 `main()`

ulong board_init_f_alloc_reserve(ulong top)
 {
 /* Reserve early malloc arena */
 \#if CONFIG_VAL(SYS_MALLOC_F_LEN)
 top -= CONFIG_VAL(SYS_MALLOC_F_LEN);
 \#endif
 /* LAST : reserve GD (rounded up to a multiple of 16 bytes) */
 top = rounddown(top-sizeof(struct global_data), 16);

​	return top;
 }啥意思

这个函数从 `top` 开始，**向低地址依次预留早期 malloc 区和 `gd` 结构体**，最后返回 `gd` 的起始地址。它只计算地址，尚未初始化这些内存。

以你的 RK3506 SPL 为例，传入 `top = 0x03F00000`：

```
高地址
0x03F00000  ← 原始 top；早期 malloc 的上沿
             早期 malloc，大小 0x100000（1 MiB）
0x03E00000  ← 减去 CONFIG_SPL_SYS_MALLOC_F_LEN 后的 top
             gd_t 及最多 15 字节的对齐空隙
G_SPL       ← 返回值 = rounddown(0x03E00000 - sizeof(gd_t), 16)
             后续栈从这里向低地址增长
低地址
```

`CONFIG_VAL(SYS_MALLOC_F_LEN)` 会选取当前构建阶段的配置值。`rounddown(..., 16)` 把 `gd` 起点向下对齐到 16 字节边界。因此返回值同时是这片预留空间的最低地址、`gd` 的地址；在你贴过的 `crt0.S` 中，它随后被赋给 `r9` 和 `sp`。`sizeof(gd_t)` 未知时，`G_SPL` 不能写成确定的十六进制地址。

```
  Worked for 13s • 16:35
```

\#ifdef CONFIG_ARM64
 asm volatile("mrs %0, cntfrq_el0" : "=r" (gd->arch.timer_rate_hz));
 \#else
 asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (gd->arch.timer_rate_hz));
 \#endif这是干嘛？

这段代码是把 **ARM 通用定时器的计数频率**读出来，保存到 `gd->arch.timer_rate_hz`，供 U-Boot 把计数值换算成时间。

- 编译为 ARM64 时，`mrs` 读取 `CNTFRQ_EL0`。
- 编译为 32 位 ARM 时，`mrc p15, 0, ..., c14, c0, 0` 读取对应的 `CNTFRQ` 寄存器。你的 RK3506 这份 32 位 U-Boot 走 `#else`。
- `"=r"` 表示让编译器用一个通用寄存器接收读出的值，再写入 `gd->arch.timer_rate_hz`。

例如读到 `24000000`，就是计数器每秒计 2400 万次，约 24 个计数对应 1 微秒。**它读取的是频率说明，不是当前计数值**；也没有在这里设置时钟源或改变频率。当前计数值要读 `CNTPCT` 或 `CNTVCT`。

```
  Worked for 10s • 16:42
```

arch.timer_rate_hz。读这个干嘛，CPU频率还是？

不是 CPU 频率。`gd->arch.timer_rate_hz` 是 ARM 通用定时器的**计数频率**，单位是“每秒计数多少次”。

U-Boot 读计数器时只得到一个数字；要计算经过了多久，就需要这个频率。例如它是 24,000,000 Hz，两次读数相差 24,000，就表示经过了 1 毫秒。延时和超时判断会用到它。

CPU 频率由 CPU 时钟和 PLL 决定，可能是几百 MHz 或更高，也可能动态变化；这里的 24 MHz 通常是定时器的参考频率，不能当作 CPU 主频。`CNTFRQ` 寄存器存的是频率值，本段代码只是读取它，并不测量或设置频率。

```
  Worked for 9s • 16:43
```

为啥要计算经过了多久，就需要这个频率？

因为计数器给出的只是“计了多少下”，并不直接给出秒数。要知道这些计数相当于多久，必须知道它每秒计多少下：

```
经过时间（秒）= 两次计数的差值 ÷ timer_rate_hz
```

例如计数差是 `24000`，频率是 `24000000 Hz`，结果就是 `0.001 秒`（1 毫秒）。如果不知道频率，同样的 `24000` 次计数，在 12 MHz 下却是 2 毫秒，无法判断实际经过的时间。

```
  Worked for 9s • 16:44
```

是因为现在还是uboot早期时钟没初始话只能通过这种方式延时计时间之类的？

不完全是。关键不是“U-Boot 早期时钟还没初始化”，而是 **CPU 主频和定时器频率是两回事**。

U-Boot 用 ARM 通用定时器计时：读 `CNTPCT`/`CNTVCT` 得到计数，再用 `CNTFRQ` 中的每秒计数次数换算成微秒，供 `udelay()`、超时判断等使用。即使 CPU 时钟已经初始化，仍然需要知道定时器频率；CPU 主频后来变化，也不应拿它来换算定时器计数。

前提是定时器本身已有时钟，而且 `CNTFRQ` 已被前面的启动阶段正确设置。你这里读取 `CNTFRQ`，**并不是在初始化时钟**；若它尚未设置，读出的频率就不能可靠用于延时。

```
  Worked for 12s • 16:45
```

这个类似于cortex-M的系统滴答定时器systick？

```
■ Conversation interrupted - use /feedback if something went wrong
```

这个类似于cortex-M的系统滴答定时器systick？他不是片上外设？

对，**用途上类似 Cortex-M 的 SysTick**：都是硬件计数器，可以用来计算经过的时间、实现延时或触发定时中断。

它也确实是**片上定时硬件**。区别在于，SysTick 是 Cortex-M 内核自带的一个向下计数定时器；这里的 ARM 通用定时器包含持续运行的系统计数器，以及各 CPU 核使用的定时比较器。`CNTFRQ` 记录系统计数器每秒计多少次，`CNTPCT`/`CNTVCT` 才是读当前计数。你贴的代码只读取频率，类似先弄清 SysTick 的计数时钟是多少，并没有启动或配置定时器。

所以你可以把它理解为“ARMv7 平台用于计时的片上硬件”，但不要把它的频率当成 CPU 主频。

```
  Worked for 13s • 16:47
```

config ROCKCHIP_PRELOADER_ATAGS
 bool "Rockchip pre-loader atags"
 default y if ARCH_ROCKCHIP
 help
   This enable support Rockchip atags among pre-loaders, i.e. ddr, miniloader, ATF,
   tos, U-Boot, etc. It delivers boot and configure information, shared with pre-loaders
   and finally ends with U-Boot.config ROCKCHIP_PRELOADER_SERIAL
 bool "Rockchip pre-loader serial"
 default y if ROCKCHIP_PRELOADER_ATAGS
 help
   This enable U-Boot using pre-loader atags serial configure to initialize console.
   It denpends on serial aliases to find pre-loader serial number.这是啥

我会查一下这两个选项在 RK3506 的 U-Boot 中具体在哪里使用，再说明 ATAGS 传递什么信息，以及串口配置如何生效。

```
• Ran rg -n "ROCKCHIP_PRELOADER_ATAGS|ROCKCHIP_PRELOADER_SERIAL|preloader.*atags|atags.*serial" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linu…
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\param.c:420:#if defined(CONFIG_ROCKCHIP_PRE…
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\param.c:421:    defined(CONFIG_ROCKCHIP_PRE…
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\param.c:450:#ifdef CONFIG_ROCKCHIP_PRELOADE…
• Ran rg -n "ATAGS_OFFSET|ATAGS_SIZE|atags_get|atags_set|ATAG_SERIAL|ATAG.*UART" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1…
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\param.c:424:    t = atags_get_tag(ATAG_SERI…
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\param.c:453:    t = atags_get_tag(ATAG_PUB_…
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\rk1808\rk1808.c:156:    t = atags_get_tag(A…
• Ran rg -n "pre-loader atags|pre-loader serial" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot" -g "Kconfig*" -g "*…
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\Kconfig:1053:    bool "Rockchip pre-loader …
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\Kconfig:1056:      This enable U-Boot using…
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\Kconfig:1057:      It denpends on serial al…
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └ 
        /* The highest priority to turn off (override) console */
    #if defined(CONFIG_DISABLE_CONSOLE)
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └     return 0;
    }
    
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └       This enable tiny image generation
    
    config ROCKCHIP_FIT_IMAGE_PACK
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └         char name[8];
            u64 phy_addr;
            u32 size;
```

源码里，SPL 会从 `ATAG_SERIAL` 读取串口启用状态、寄存器地址、波特率和编号；没有这个标签时会回退到编译时配置。我再核对串口编号与设备树 alias 的匹配位置。

```
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └ 
        return (t->hdr.magic == ATAG_CORE);
    }
• Ran rg -n "using_pre_serial|serial\.id|serial\.addr|serial\.enable|serial\.baudrate" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\drivers\ram\rockchip\sdram_rk3308.c:664:    t_serial.id = 2;
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\drivers\ram\rockchip\sdram_rk3308.c:668:    t_serial.id = 3;
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\drivers\ram\rockchip\sdram_rk3308.c:672:    t_serial.id = 4;
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └ #endif
    
    #if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
• Exited (status 1) rg -n "CONFIG_ROCKCHIP_PRELOADER_ATAGS|CONFIG_ROCKCHIP_PRELOADER_SERIAL" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.…
  └ (no output)
• Ran rg -n "serial[0-9]|of_alias|alias.*serial|gd->serial.id|serial.id" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-b…
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\drivers\serial\serial-uclass.c:43:         *    stdout-path = "ser…
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └     struct NS16550 *com_port;
    
        if (gd && gd->flags & GD_FLG_DISABLE_CONSOLE)
```

这两个选项管的是**前一级启动程序向 U-Boot 交接信息**。

`ROCKCHIP_PRELOADER_ATAGS`：启用瑞芯微的 ATAGS 交接区。前一级程序（例如 DDR 初始化程序、miniloader）把信息写成带类型的标签，后面的 SPL/U-Boot 从中读取。标签可以包含 DDR 容量、启动设备、串口设置等。你的 RK3506 配置把这块区域放在 DDR 的 `0x00062000–0x00062FFF`，大小 4 KiB，见 rk3506_common.h (E:/BaiduNetdiskDownload/TL3506-MiniEVM/4-软件资料/Linux/LinuxSDK/rk3506_linux6.1_release_v1.1.0/u-boot/include/configs/rk3506_common.h:34)。这也应该作为一块占用区标在之前的 DDR 图上。

`ROCKCHIP_PRELOADER_SERIAL`：允许 U-Boot 从其中的 `ATAG_SERIAL` 继承串口配置。在这份源码里，它读取 `enable`、串口寄存器地址 `addr`、`baudrate` 和编号 `id`；若 `enable=0`，会关闭控制台输出；若没有该标签，则使用编译时的 `CONFIG_BAUDRATE` 和 `CONFIG_DEBUG_UART_BASE`，见 spl.c (E:/BaiduNetdiskDownload/TL3506-MiniEVM/4-软件资料/Linux/LinuxSDK/rk3506_linux6.1_release_v1.1.0/u-boot/arch/arm/mach-rockchip/spl.c:287)。帮助文字提到的 serial aliases，是设备树中用于对应串口编号的别名。

`default y if ...` 只表示满足条件时**默认开启**，并非强制开启。这里的 ATAGS 是启动阶段之间的共享数据格式，别与“把内核启动参数传给 Linux”的旧式 ARM ATAGS 用途混为一谈。

```
  Worked for 1m 1s • 16:50
```

ATAGS全称是啥，还有那你内存分布图画了 ATAGS 交接区吗？

ATAGS 通常展开为 “ARM Tags”，也就是 ARM 的标签列表。上次图里确实漏画了 RK3506 的 `0x00062000–0x00062FFF` 交接区；我现在把它加到 HTML 和 draw.io 图中，并标出两个边界宏。

```
• Ran Select-String -LiteralPath 'D:\RK3506_DDR_memory_map.html' -Pattern 'ATAGS|0x00063000|0x00008000|0x00000000|address-index' | Select-Object LineNumber,Line
  └        128       <li class="row limit"><div class="address">0x00000000</div><div class="detail"><strong>CONFIG_SYS_S...
    
    
• Ran Select-String -LiteralPath 'D:\RK3506_DDR_memory_map.drawio' -Pattern 'ATAGS|0x00063000|0x00008000|0x00000000|id="fdt"|id="load"' | Select-Object LineNumber,Line
  └         20         <mxCell id="base" value="0x00000000　CONFIG_SYS_SDRAM_BASE：DDR 起始地址" style="rounded=0;whiteSpace=w...
    
    
• Ran Get-Content -LiteralPath 'D:\RK3506_DDR_memory_map.drawio' -Tail 15
  └     </root></mxGraphModel>
      </diagram>
    </mxfile>
• Ran Get-Content -LiteralPath 'D:\RK3506_DDR_memory_map.drawio' -Encoding UTF8 | Select-Object -Skip 60 -First 16
  └       <mxCell id="a2" value="0x03F40000" style="text;html=1;fontSize=14;fontStyle=1;strokeColor=none;fillColor=none;" vertex="1" parent="1"><mxGeometry x="555" y="238" w…
          <mxCell id="z2" value="SPL 镜像：最多 256 KiB" style="text;html=1;fontSize=13;align=center;fillColor=#DCFCE7;strokeColor=none;" vertex="1" parent="1"><mxGeometry x…
          <mxCell id="p3" value="SPL_TEXT_BASE / SPL_STACK →" style="text;html=1;fontSize=13;align=right;strokeColor=none;fillColor=none;" vertex="1" parent="1"><mxGeometry …
```

我会把 ATAGS 画成从 `CONFIG_SYS_SDRAM_BASE + ATAGS_OFFSET = 0x00062000` 到 `+ ATAGS_SIZE = 0x00063000` 的 4 KiB 区间。`fdt_addr_r = 0x00063000` 恰好紧挨着它的上边界，也会在图上说明。

```
• Edited RK3506_DDR_memory_map.html (+11 -0)
+    <section aria-labelledby="atags-title">
+      <h2 id="atags-title">Rockchip ATAGS：启动阶段交接区</h2>
+      <div class="boundaries" aria-label="ATAGS 的起止地址与占用区">
• Ran Get-Content -LiteralPath 'D:\RK3506_DDR_memory_map.drawio' -Encoding UTF8 | Select-Object -Skip 15 -First 8
  └         <mxCell id="note" value="第二页：早期预留。第三页：重定位预留。第四页：固定地址的指针和分界线。&#xa;各启动阶段先后执行，DDR 地址可以在后续阶段复用；所有高度均不…
          </root>
        </mxGraphModel>
```

HTML 图已加上两个边界。接下来同步更新 draw.io 的总览页和分界线页，让两份图一致。

```
• Edited RK3506_DDR_memory_map.drawio (+13 -3)
-        <mxCell id="load" value="0x00008000　CONFIG_SYS_LOAD_ADDR：默认文件加载地址，可被命令覆盖" style="rounded=0;whiteSpace=wrap;html=1;fontSize=14;align=left;spacingLeft=14;fillColor=#E0F2FE;strokeColor=#0284C7;" vertex="1" parent="1"><mxGeometry x="40" y="571" width="730" height="43" as="geometry"/></mxCell>
-        <mxCell id="base" value="0x00000000　CONFIG_SYS_SDRAM_BASE：DDR 起始地址" style="rounded=0;whiteSpace=wrap;html=1;fontSize=14;align=left;spacingLeft=14;fillC
• Ran Select-String -LiteralPath 'D:\RK3506_DDR_memory_map.html','D:\RK3506_DDR_memory_map.drawio' -Pattern 'ATAGS_PHYS_BASE|0x00062000|0x00062FFF|0x00063000' | Select-Objec…
  └ D:\RK3506_DDR_memory_map.drawio        110
    
    
• Ran [xml]$d=Get-Content -LiteralPath 'D:\RK3506_DDR_memory_map.drawio' -Encoding UTF8 -Raw; $d.mxfile.diagram | Select-Object id,name
  └ boundary-pointers 固定地址分界指针         
    
    
```

ATAGS 通常指 **ARM Tags**，即 ARM 的标签列表。这里是瑞芯微用于启动阶段之间传递信息的标签区。

之前的图确实漏画了它；现在已补到 Edge 可打开的内存图 (D:/RK3506_DDR_memory_map.html) 和 draw.io 图 (D:/RK3506_DDR_memory_map.drawio)：`ATAGS_PHYS_BASE = 0x00062000`，占用 `0x00062000–0x00062FFF`，共 4 KiB。上边界 `0x00063000` 恰好是 `fdt_addr_r`，但不属于 ATAGS 区。两份图均已核对，draw.io XML 可正常解析。

```
  Worked for 1m 28s • 16:53
```

help
   The secure timer inited in SPL/TPL in secure word, ARM generic timer
   works after this timer work.这是啥

我先查这段 help 对应的配置项和初始化代码，再说明这里的 “secure world” 指什么，以及它与 ARM 通用定时器的关系。

```
• Exited (status 1) rg -n "TIMER.*SECURE|SECURE.*TIMER|CNTFRQ|cntfrq|CNTPCT|CNTVCT" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release…
  └ (no output)
• Ran rg -n -C 9 "secure timer inited|ARM generic timer|secure word" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot"…
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\Kconfig-799-    default 0x10080000 if ROCKC…
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\Kconfig-800-    default 0x10080000 if ROCKC…
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\Kconfig-801-    default 0x10080000 if ROCKC…
• Ran rg -n "CONFIG_ROCKCHIP_TIMER|ROCKCHIP_TIMER|TIMER_BASE|SECURE_TIMER" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u…
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\rk3506\rk3506.c:111:    writel(0xffffffff, …
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\rk3506\rk3506.c:112:    writel(0xffffffff, …
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\rk3506\rk3506.c:113:    writel(0x00010001, …
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └ config ROCKCHIP_IRAM_START_ADDR
        hex "Rockchip Secure timer base address"
        default 0xff0e0000 if ROCKCHIP_PX30
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └     return 0;
    }
    
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └ #endif
    
    int arch_cpu_init(void)
• Ran rg -n -C 3 "rockchip_stimer_init\(" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip" -g …
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\rk3576\rk3576.c-155-{
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\rk3576\rk3576.c-156-    u32 reg;
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\mach-rockchip\rk3576\rk3576.c-157-
```

这段 help 说的是：**先在早期启动阶段初始化瑞芯微的安全定时器，后续才能可靠使用 ARM 通用定时器计时。**其中 `secure word` 应是 `secure world`（安全世界）的笔误。

对你的 RK3506，`ROCKCHIP_STIMER_BASE` 是安全定时器的寄存器基址 `0xff980000`，不是 DDR 地址，见 Kconfig (E:/BaiduNetdiskDownload/TL3506-MiniEVM/4-软件资料/Linux/LinuxSDK/rk3506_linux6.1_release_v1.1.0/u-boot/arch/arm/mach-rockchip/Kconfig:766)。初始化代码 (E:/BaiduNetdiskDownload/TL3506-MiniEVM/4-软件资料/Linux/LinuxSDK/rk3506_linux6.1_release_v1.1.0/u-boot/arch/arm/mach-rockchip/rk3506/rk3506.c:101) 会先检查它是否已启用；若未启用，就写入 `CNTFRQ = 24000000`，设置定时器寄存器并启用它。若前一级已经启用，SPL 会直接跳过，避免重新计时。

你前面看到的 `gd->arch.timer_rate_hz`，是在**读取** `CNTFRQ` 保存频率；这里则是在更早阶段**写入** `CNTFRQ` 并启动相关定时硬件。两段代码处于计时初始化的不同步骤。

```
  Worked for 57s • 17:23
```

0xff980000就是高精度定时器，我看芯片手册写的，就是拿个高精度定时器外设来当安全定时器？

你指出了芯片手册里的硬件名称。我会核对 RK3506 的定时器节点和这段寄存器初始化代码，确认“高精度定时器”和代码里的 `stimer` 是什么关系。

```
• Ran [Console]::OutputEncoding=[System.Text.Encoding]::UTF8; Get-Content -LiteralPath "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_rele…
  └     if (!loader)
            return;
    
• Exited (status 1) rg -n -i "ff980000|hptimer|high.precision.timer|secure.timer" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v…
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\dts\rk3528.dtsi:1281:        reg = <0x0 0xff980000 0x0 0x…
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\dts\rv1106.dtsi:894:    rga2: rga@ff980000 {
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\dts\rv1106.dtsi:896:        reg = <0xff980000 0x1000>;
• Exited (status 1) rg -n -i "ff980000|hptimer|high.precision.timer" "E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0" -g "*.…
  └ E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\dts\rk3528.dtsi:1281:        reg = <0x0 0xff980000 0x0 0x…
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\dts\rk3288.dtsi:833:    hdmi: hdmi@ff980000 {
    E:\BaiduNetdiskDownload\TL3506-MiniEVM\4-软件资料\Linux\LinuxSDK\rk3506_linux6.1_release_v1.1.0\u-boot\arch\arm\dts\rk3288.dtsi:835:        reg = <0xff980000 0x20000>;
```

对，你按 RK3506 手册理解更准确：`0xff980000` 是芯片的高精度定时器外设寄存器地址；瑞芯微在 U-Boot 中把这块用于早期安全环境计时的硬件称为 `stimer`（secure timer）。这里的“安全”指启动阶段的安全运行环境，不是说它负责加密。

在 RK3506 初始化代码 (E:/BaiduNetdiskDownload/TL3506-MiniEVM/4-软件资料/Linux/LinuxSDK/rk3506_linux6.1_release_v1.1.0/u-boot/arch/arm/mach-rockchip/rk3506/rk3506.c:101) 中，SPL 检查这个外设是否已启用；未启用才配置并启动它，同时把 ARM 的 `CNTFRQ` 写为 24 MHz。所以可以理解为：**初始化 RK3506 的高精度定时器硬件，让后续 ARM 通用定时器计时可用**。我前面把“安全定时器”说得像另一块独立外设，表述不准确。

```
  Worked for 53s • 17:25
```

asm volatile("mcr p15, 0, %0, c14, c0, 0"
 	     : : "r"(COUNTER_FREQUENCY));
 这是干嘛

这条汇编把 `COUNTER_FREQUENCY` 写入 ARM 通用定时器的频率寄存器 `CNTFRQ`。在你的 RK3506 配置中，该值是 `24000000`，表示计数器每秒计数 2400 万次。

`mcr p15, 0, %0, c14, c0, 0` 是 32 位 ARM 写协处理器寄存器的指令；`"r"(COUNTER_FREQUENCY)` 让编译器把这个值放进通用寄存器，再由 `%0` 引用。前面的空输出列表 `: :` 表示只有输入，`volatile` 防止编译器删掉这次写入。

**关键区别：这条指令只写“计数频率是多少”，不会改变硬件实际频率，也不会单独启动定时器。** 后面的 `writel(...)` 才是在配置 RK3506 的高精度定时器外设。U-Boot 稍后读取 `CNTFRQ` 到 `gd->arch.timer_rate_hz`，用它把计数差换算成时间。

```
  Worked for 11s • 17:26
```

asm volatile("mcr p15, 0, %0, c14, c0, 0"
 	     : : "r"(COUNTER_FREQUENCY));
 writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x14);
 writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x18);
 writel(0x00010001, CONFIG_ROCKCHIP_STIMER_BASE + 0x4);，所以这条asm都作用是？

这条 `asm` 的作用是**告诉 ARM 通用定时器：计数频率是 24 MHz**。它把 `COUNTER_FREQUENCY` 写进 `CNTFRQ` 寄存器，供后面的 U-Boot 把计数值换算成时间。

结合下面三条 `writel` 看：它们设置并启用 RK3506 的高精度定时器；`asm` 则写入与该计时硬件相对应的频率值。**`asm` 本身不启动定时器，也不把硬件时钟调到 24 MHz。** 如果实际计数频率与写入的 24 MHz 不一致，延时和超时计算就会不准。