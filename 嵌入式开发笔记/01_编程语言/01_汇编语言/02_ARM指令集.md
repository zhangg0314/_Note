# 基本指令

## 1.数据处理指令

### 1.数据搬移指令

```asm
MOV{条件}{S}  <dest>, <op 1> @ dest = op_1
MVN{条件}{S}  <dest>, <op 1> @ dest = !op_1

@{条件}：
条件码后缀
@{S}：
影响条件位寄存器的后缀
@<dest>:
目标寄存器，只能是寄存器
@<op 1>：
第1操作数，可以是一个寄存器、一个被移位的寄存器、或一个立即数

@eg：
MOV R2,R1
MOV R1,#1

@立即数：
本质就是包含在指令当中的数，是指令的一部分。不像变量那样独占一个空间，故读取时在读指令的时候就读了，不必单独访存，因此速度更快。但缺点就是：不能是任意的32位的数字，有局限性。

MOV R0,#0X12345678	//机器码才32位，一个占32位的数都超了肯定报错。
MOV R0,#0X12		/把能够放到寄存器的数叫做立即数。

@MVN:
MVN R0，#0xFF		//先把0xFF按位取反，再搬到R0，R0=~0xFF
MOV R0,#0XFFFFFFFF	 //虽然数值远远大，但编译不报错，因为它会被翻译成MVN，只要效果一样即可。
```
### 2.数据运算指令

```asm
@数据运算指令格式:
操作码{条件}{S}  <dest>, <op 1>, <op 2>

@{条件}：
条件码后缀
@{S}：
影响条件位寄存器的后缀
@<dest>:
目标寄存器,只能是寄存器
@<op 1>
第一操作寄存器，只能是寄存器
@<op 2>
第二操作数，可以是一个寄存器，被移位的寄存器，或一个立即值@乘法指令除外

@eg:
@1.ADD 加
    	ADD R1,R2,R3 @R1 = R2 + R3
    	ADD R1,R2,#5 @可以
    	ADD R1,#5,#6 @error
    	ADD R1,#5,R2 @error
 @2.SUB 减
    	SUB R1,R2,R3
    	@R1 = R2 - R3
 @3.RSB 逆向减法，解决SUB不能实现R1 = 立即数 - R2的问题
    	RSB R1,R2,#3 
    	@R1 = 3 - R2
 @4.MUL 乘法指令
    	MUL R1,R2,R3
    	@R1 = R2 * R3
    	MUL R1,R2,#3//error，乘法指令不能用立即数
```

### 3.位运算

```asm
@位指令格式:
操作码{条件}{S}  <dest>, <op 1>, <op 2>

@{条件}：
条件码后缀
@{S}：
影响条件位寄存器的后缀
@<dest>:
目标寄存器,只能是寄存器
@<op 1>
第一操作寄存器，只能是寄存器
@<op 2>
第二操作数，可以是一个寄存器，被移位的寄存器，或一个立即值@（BIC除外）

@1.AND 按位与
    AND R1,R2,R3 @R1 = R2 & R3
@2.ORR 按位或
	ORR R0, R0, #3 @设R0 = R0 | 3
@3.EOR 按位异或
	EOR R0, R0, #3 @R0 = R0 ^ 3
@4.BIC 位清0
    BIC R1,R2,#0xF #R2本身不变，R1的值变为R2的某些位被清0后的结果，某些位是指#0xFF哪些位为1，就清哪些位,即BIC的第二操作数为mask掩码
```

### 4.带S的数据运算指令

```asm
@默认情况下数据运算不会对条件位参数(N,Z,C,V)产生影响，当在指令后加后缀S后可以影响。
@eg:
MOV R1,#3
SUBS R2,R1,#5  @实质还是R2 = 3 - 5，但N.Z.C.V会被置位置

@SBC：Subtraction with Carry
	SBC R6,R2,R4
		R6 = R2 - '!c' - R4@会带借位减。但不会影响条件位
		
@总结
ADDS仅影响条件位
ADC仅计算时带进位
ADCS不仅计算时带进位，而且计算后的结果也影响条件位。

@eg：
	ADC R6,R2,R4
		R6 = R2 + R4 + 'C'

@64bit分两次算，一次只能算32bit
```

## 2.跳转指令

### 1.程序跳转本质

实现程序的跳转，本质就是修改PC寄存器。

### 2.跳转方式

#### 1.直接修改PC的值

```asm
MOV PC,#0x18
@但需要我们自己去计算绝对地址，很麻烦。
```

#### 2.不带返回的跳转指令

```asm
MAIN:
	......
	B FUNC
	......
FUNC:
	......
	......
@本质就是将PC寄存器的值修改成跳转标号下第一条指令的地址。
```

#### 3.带返回的跳转指令

```asm
BL FUNC
@本质是将PC寄存器的值修改成跳转标号下第一条指令的地址，同时将跳转指令下一条指令的地址存储到当前模式下的LR
```

#### 4.带条件的跳转，但不带返回

```asm
CMP R1,R2
BEQ FUNC 
@CPU先看Z是否置1，如果是，则跳转FUNC
@EQ为条件码
@不带返回
```

## 3.移位指令

​	移位操作在 ARM 指令集中**不作为单独的指令使用**，它是指令格式中是一个字段，在汇编语言中表示为指令中的选项。

```asm
@6大助记符
LSL  逻辑左移
ASL  算术左移
LSR  逻辑右移
ASR  算术右移
ROR  循环右移
RRX  带扩展的循环右移


@1.LSL 逻辑左移
   	LSL R1,R2,R3 
   		@R1 = (R2 << R3)
@5.LSR 逻辑右移
	MOV R1,R2,LSL,#1 
		@R1 = (R2 << 1)
```

------

# ARM条件码

是各种指令操作码的后缀，ARM大多数指令均可以加后缀。![image-20240722163853728](..\..\figure\image-20240722163853728.png)

```asm
CMP R1,R2
@比较R1,R2的数，本质是SUBS R1,R2，并根据比较结果设置N,Z,C,V
BEQ FUNC
MOVEQ R3,#3
```

------

# 访存指令

## 1.Load/Store指令

```asm
@访问（读写）内存

@读内存：
LDR R3,[R2] @将内存中R2指向的内存空间中的数据读取到R3

@写内存
STR R1,[R2] @将寄存器的R1的数存在R2指向的内存空间，也验证了大小端问题


STRB R1,[R2]@只存一个字节,优先写低位
STRH R1,[R2]@HALFWORD
STR R1,[R2] @默认是一个字
```

## 2.ARM指令寻址方式

寻址方式就是CPU去寻找一个数的操作方式。

- **立即寻址**
  也叫立即数寻址，数来自于机器码。
- **寄存器寻址**
  数来自寄存器。
- **寄存器移位寻址**
  数来自一个寄存器中的数做了移位操作后的结果。
- **寄存器间接寻址**
  数来自内存。
- **基址加变址寻址**
  [R2,R3]数来自内存中`地址==[R2] + [R3]`的地方。
- **基址加变址寻址的索引方式**
  前索引，后索引，自动索引

  ```asm
  MOV R1,[R2,#8]  @ R1 = *(R2+8),R2 = R2,前索引
  MOV R1,[R2],#8  @ R1 = *(R2),R2 = R2 + 8,后索引
  MOV R1,[R2,#8]! @ R1 = *(R2+8),R2 = R2 + 8,自动索引
  ```

## 3.多寄存器内存访问指令

```asm
STM R11,{R1-R4}@将R1-R4中的值存到以[R11]为首地址的内存区域
LDM R11,{R6-R9}@将内存中以[R11]为首地址的数据读取到R6-R9寄存器
STM R11,{R1,R2,R4}@以逗号分隔不连续的情况
STM R11,{R2,R4,R1}@不管寄存器列表中的顺序如何，存储时永远是低地址存储小编号的寄存器

@自动索引照样适用于多寄存器内存访问指令
STM R11!,{R1-R4} @R11自动增，增的量为存的数据的总大小，如存16个字节数据，地址加增加16


@多寄存器内存访问指令寻址方式
STMIA R11!,{R1-R4}@即默认情况，等价于不加后缀，IA，increase after
STMIB R11!,{R1-R4}@先在指定地址基础上加4个字节，然后再开始存，IB，increase before
STMDA R11!,{R1-R4}@D,decrease
STMDB R11!,{R1-R4}
```

------

# 栈

## 1.概念

SP寄存器的内容存放的就是栈指针的地址，而栈的本质就是一段内存，程序运行时用于保存一些临时数据。如局部变量，函数参数，返回值等。

## 2.栈的分类

针对于压栈时的特点有如下分类：

- 增栈--存数据时，SP往高地址方向移动。

- 减栈--存数据时，SP往低地址方向移动。

- 满栈：栈指针指向最后一次压入到栈中的数据，压栈时需要先移动栈指针到相邻位置再压栈

- 空栈：栈指针指向最后一次压入到栈中的数据的相邻位置，压栈时可以直接压栈，之后需要将栈指针移动到相邻位置。
- EA（空增栈）,ED,FA,FD（满减栈），**ARM一般使用满减栈，用STMDB进行压栈，STMIA出栈**

```asm
STMFD @等价于 STMDB，更简单
LDMFD @等价于 STMIA，更简单
```

## 3.应用

```asm
@初始化栈
	MOV SP,#0x40000000
MAIN:
	MOV R1,#3
	MOV R2,#5
	BL FUNC 
	ADD R3,R1,R2
	B STOP
FUNC:
	@压栈保护线程
	STMFD SP!,{R1,R2}
	MOV R1,#10
	MOV R2,#20
	SUB R3,R2,R1
	@出栈恢复现场
	LDMFD SP!,{R1,R2}
	MOV PC,LR
STOP:
	B STOP
@叶子函数，位于函数调用递归树的最末端，对于叶子函数，不需要进行LR的压栈保护，对于非叶子函数要压栈保存通用寄存器和LR寄存器的值。

@出栈时，内存中该位置的值并未被清除，仅仅只是栈指针移动，又因为C语言局部变量也是压入栈中，故局部变量不初始化会有不确定的值。
```

# ARM专有指令

与C语言无关，即C语言不会被编译成的汇编指令。

## 1.状态寄存器传送指令

用于访问（读写）CPSR寄存器。

```asm
@读CPSR
MRS R1,CPSR @R1 = CPSR

@写CPSR
MSR CPSR,#0x10 

@在USER模式下不能修改CPSR,非特权模式
```

## 2.软中断指令

用于给`cpu`发生软中断信号。

```asm
SWI #1
@软中断指令：触发软中断
	@异常向量表
	B MAIN
	B .
	B SWI_HANDLER     @0x08
	B .
	B .
	B .
	B .
	B .
@应用程序
MAIN:
	MSR CPSR,#0x10
	MOV R1,#1
	MOV R2,#2
	SWI #1     @置PC为异常向量表的SWI即0x08地址
	ADD R3, R2,R1
	B STOP
@异常处理程序
SWI_HANDLER:
	@压栈保护现场
	STMFD SP! {R1,R2,LR}
	...
	...
	@出栈恢复现场
	...
	LDMFD SP! {R1,R2,PC}^ @^的作用就是把SPSR赋值给CPSR

STOP:
	B STOP
```

## 3.协处理器指令

用于操控协处理器的指令。

```asm
@1.协处理器数据运算指令
	CDP
	
@2.协处理器存储器访问指令

	STC @将协处理器中的数据存到存储器
	LDC @将存储器中的数据读取到协处理器
	
@3.协处理器寄存器传送指令
	MRC @协处理器的寄存器到ARM的寄存器
	
	MCR @Move to Coprocessor from ARMRegister，ARM的寄存器协到处理器的寄存器
	MCR{条件} <coproc>，<op 1>，<Rd>，<CRn>，<CRm> {<op 2>}
	@<coproc>
		指定协处理器的编号，标准的协处理器的名字为 p0、p1、…、p15。
	@<opcode_1>
		指定协处理器执行的操作码，确定哪一个协处理器指令将被执行。
	@<Rd>
		确定哪一个 ARM 寄存器的数值将被传送。如果程序计数器（PC）的值被传送，指令的执行结果不可预知。
	@<CRn>
		确定包含第一个操作数的协处理器寄存器。
	@<CRm>
		确定包含第二个操作数的协处理器寄存器。
	@<opcode_2>
		指定协处理器执行的操作码，确定哪一个协处理器指令将被执行。
	
	@CRn + opcode_1 + CRm + opcode_2 组成的序号唯一标识协处理器中的某个寄存器
```

## 4.伪指令

```asm
NOP @空指令，等价于MOV R0,R0，虽然不执行，但也消耗一个指令周期
	
LDR R1,[R2] @此格式表示的是指令
LDR R1,=0x12345678 
@此格式表示的是伪指令，等价于@R1 = 0x12345678，
@可以将任意一个32位数据放到寄存器中，解决MOV指令不能将任意32位数搬到寄存器中去的问题

LDR R1,=STOP @将STOP代码段的第一条语句的地址写入R1寄存器，给的是指令地址
STOP:
	B STOP
	
LDR R1,STOP @将STOP代码段的第一条指令的机器码写入R1寄存器，给的是指令内容本身
```

## 5.伪操作

### 1.特性

```asm
@伪操作一般以'.'开头
```

### 2.全局局部属性

```asm
@1.声明全局标识
	.global symbol_name
	.globl 	symbol_name
	#将一个如MAIN,STOP这样的符号定义为全局符号，使得链接器能够全局识别它，即一个程序文件中定义的符号能够被所有其他程序文件可见。

@2.声明局部标识
	.local symbol_name  
	#将一个符号定义为局部符号，使得此符号不能够被其他程序文件可见。
```

### 3.属性弱化

```asm
@1.弱化属性
	.weak symbol_name
	#在汇编程序中，符号的默认属性为强（strong），.weak伪操作则用于设置符号的属性为弱（weak），如果此符号之前没有定义过，则同时创建此符号并定义其属性为weak。
```

如果符号的属性为weak，那么它无需定义具体的内容。在链接的过程中，另外一个属性为strong的同名符号可以将此weak符号的内容强制覆盖。利用此特性，.weak伪操作常用于预先预留一个空符号，使得其能够通过汇编器语法检查，但是在后续的程序中定义符号的真正实体，并且在链接阶段将空符号覆盖并链接。

### 4.类型指定

```asm
.type  symbol_name , type description

#.type伪操作用于定义符号的类型。
#譬如“.type symbol,@function”即将名为symbol的符号定义为一个函数（function）。
```

### 5.地址对齐

```asm
	.align n
	#让下一条语句地址对齐到地址为2^n的位置,实质就是将当前PC地址推进到“2的integer次方个字节”对齐的位置。
	
	.balign n
	#伪操作用于将当前PC地址推进到“n个字节”对齐的位置。
	
	.zero n
	#zero伪操作将从当前PC地址处开始分配n个字节空间并且用0值填充。
```

### 6.空间分配

```asm
.byte
.byte expression [, expression]*
.byte 0xFF
#.byte伪操作将从当前PC地址处开始分配若干个字节（byte）的空间，每个字节填充的值由分号分隔开的expression指定。

.2byte
.2byte expression [, expression]*
#.2byte伪操作将从当前PC地址处开始分配若干个双字节（2 bytes）的空间，每个双字节填充的值由分号分隔开的expression指定。空间分配的地址可以与双字节非对齐。

.4byte
.4byte expression [, expression]*
#.4byte伪操作将从当前PC地址处开始分配若干个四字节（4 bytes）的空间，每个四字节填充的值由分号分隔开的expression指定。空间分配的地址可以与四字节非对齐。

.8byte
.8byte expression [, expression]*
#.8byte伪操作将从当前PC地址处开始分配若干个八字节（8 bytes）的空间，每个八字节填充的值由分号分隔开的expression指定。空间分配的地址可以与八字节非对齐。

.half
.half expression [, expression]*

#.half伪操作将从当前PC地址处开始分配若干个半字（half-word）的空间，每个半字填充的值由分号分隔开的expression指定。空间分配的地址一定与半字对齐（half-word aligned）。

.word
.word expression [, expression]*
#.word伪操作将从当前PC地址处开始分配若干个字（word）的空间，每个字填充的值由分号分隔开的expression指定。空间分配的地址一定与字对齐（word aligned）。

.dword
.dword expression [, expression]*
#.dword伪操作将从当前PC地址处开始分配若干个双字（double-word）的空间，每个双字填充的值由分号分隔开的expression指定。空间分配的地址一定与双字对齐（double-word aligned）。

.string
.string “string”
#.string伪操作将从当前PC地址处开始分配若干个字节空间用于存放“string”字符串。字节的个数取决于字符串的长度。

.assicz
#.assicz汇编指令用来向当前段添加字符串。字符串是一串字节序列表示。.string和.asciz会在字符串末尾额外添加一个0值的字节，即NULL结尾的字符串。

.assic
#.assic汇编指令用来向当前段添加字符串

.float
.float 或者 .double expression [, expression]*

#.float伪操作将从当前PC地址处开始分配若干个单精度浮点数（32位）的空间，每个单精度浮点数填充的值由分号分隔开的expression指定。空间分配的地址一定与32位对齐。

#.double伪操作将从当前PC地址处开始分配若干个双精度浮点数（64位）的空间，每个双精度浮点数填充的值由分号分隔开的expression指定。空间分配的地址一定与64位对齐。


.comm
.comm或者.common name, length

#.comm和.common伪操作用于声明一个名为name的未初始化存储区间，区间大小为length个字节。
```



```asm


@3.声明一个宏
	.equ DATA,0xFF
		MOV R1,#DATA
@4.封装汇编指令	
.macro FUNC  #对汇编指令的封装
	......
	MOV R1,#1
	MOV R2,#2
	......
	......
.endm
		FUNC
@5.	条件编译	
.if 1         
	...
	...
	...
.endif
@6.重复n次
.rept n        
	MOV R1,#1
	MOV R2,#2	
.endr

@8.
MOV R1,#1
.word 0xFFFFFFFF     @在当前地址申请一个字的空间并将其初始化0xFFFFFFFF
MOV R2,#2

@9.
MOV R1,#1


MOV R2,#2

@9.
.space n,#0x12        @申请任意字节n，并将其初始化
```

## 6.特殊

### 1.缺省入口

```asm
_start
@汇编程序的缺省入口，但是可以更改，想要更改其他标志，到相应的链接脚本（.lds文件）中去用ENTRY指明其他入口标志。标号可以直接认为是地址
```

### 2.U-Boot的宏

`ENTRY() `和`ENDPROC()`这两个宏定义在`#include <linux/linkage.h>`中

```asm
@ENTRY(save_boot_params)展开后如下：
.globl  save_boot_params
.align  4
save_boot_params:
bx  lr

@ENDPROC(save_boot_params)展开后如下：

.type save_boot_params STT_FUNC
.size save_boot_params, .-save_boot_params
```

# U-Boot启动源码分析

下面代码是 **ARMv7 架构 U-Boot 的核心启动汇编代码**。

## 1.符号定义

首先看开头的头文件和全局符号，它们是代码运行的基础依赖：

```asm
	.globl reset              			 @声明 reset 为全局符号（复位入口，芯片上电后首条执行的指令地址）
	
	.globl save_boot_params_ret 		 @声明 save_boot_params 的返回点
	
	.type  save_boot_params_ret,%function @标记为函数类型（链接器需识别），需要遵循函数调用规范（如对齐、栈帧、符号表类型）

#ifdef CONFIG_ARMV7_LPAE    @若启用ARMv7大物理地址扩展（LPAE，支持>4GB内存）
	.global	switch_to_hypervisor_ret @声明虚拟化模式返回点
#endif
```

**作用**：引入必要的配置和架构定义，声明启动关键符号，确保代码能被链接器正确识别，且与后续 C 语言代码兼容。

## 2.从复位到跳转到 `_main`

### 1.复位入口（`reset`）

芯片上电后的第一个执行点

```asm
reset:
    b	save_boot_params       @跳转到 save_boot_params，保存启动参数，Allow the board to save important registers
save_boot_params_ret:         @save_boot_params 的返回点
```

- **背景**：ARM 芯片上电后，PC 指针会指向 `reset` 符号（链接脚本指定的复位地址），此时 CPU 处于 **复位模式（Reset Mode）**，寄存器状态未定义。
- **`save_boot_params` 的作用**：
  这是一个 **弱符号函数（`.weak save_boot_params`）**，默认实现是直接跳回 。`save_boot_params_ret`（见代码末尾）。若某块开发板需要保存特殊启动参数（如从 ROM 传递的硬件信息），可在板级代码中重写该函数（覆盖弱符号），实现定制化参数保存。

### 2.虚拟化支持检查

可选，`CONFIG_ARMV7_LPAE` 启用时调用。

```asm
#ifdef CONFIG_ARMV7_LPAE
	/* check for Hypervisor support */
	mrc	p15, 0, r0, c0, c1, 1		  	@ 读取 ID_PFR1 寄存器（CPU 功能特性寄存器1）
	and	r0, r0, #CPUID_ARM_VIRT_MASK	@ 掩码出“虚拟化支持”位（CPUID_ARM_VIRT_MASK 是预定义常量）
	cmp	r0, #(1 << CPUID_ARM_VIRT_SHIFT)@ 比较是否支持虚拟化（该位为1表示支持）
	
	beq	switch_to_hypervisor       @若支持，跳转到 switch_to_hypervisor（进入虚拟化模式）
	switch_to_hypervisor_ret:     	@虚拟化模式返回点
#endif
```

- **关键指令 `mrc`**：之前讲过，`mrc` 是从协处理器（此处为 CP15，系统控制协处理器）读取数据到 ARM 核心寄存器。`ID_PFR1` 寄存器记录 CPU 支持的功能（如虚拟化、浮点）。
- **作用**：检查 CPU 是否支持 Hypervisor（虚拟化）模式，若支持则进入该模式（用于运行虚拟机监控程序，也可在板级代码中重写该函数（覆盖弱符号）），否则跳过。这是 ARMv7 高端芯片（如 Cortex-A15）的可选功能，低端芯片可能不支持。

### 3.CPU 模式与中断配置 

进入安全的执行模式。

```asm
/* disable interrupts (FIQ and IRQ), also set the cpu to SVC32 mode,
 * except if in HYP mode already
 */
mrs	r0, cpsr          // 读取 CPSR 寄存器（当前程序状态寄存器，记录 CPU 模式、中断使能）
and	r1, r0, #0x1f     // 掩码出“模式位”（CPSR 的 bit0-bit4，共5位，代表当前 CPU 模式）
teq	r1, #0x1a         // 检查是否已在 HYP（虚拟化）模式（0x1a 是 HYP 模式的编码）
bicne	r0, r0, #0x1f    // 若不在 HYP 模式，清除原有模式位
orrne	r0, r0, #0x13    // 若不在 HYP 模式，设置为 SVC 模式（0x13，超级管理员模式，U-Boot 核心运行模式）
orr	r0, r0, #0xc0     // 禁用 FIQ（bit6）和 IRQ（bit7）——初始化阶段避免被中断打断
msr	cpsr,r0           // 将配置写回 CPSR，生效 CPU 模式和中断禁用
```

- **为什么选 SVC 模式**：
  SVC 模式是 ARM 的 **特权模式**，允许访问所有系统资源（如 CP15 寄存器、外设寄存器），是 U-Boot 初始化和运行的核心模式。
- **禁用中断的原因**：
  初始化阶段（如缓存、MMU 配置）若被中断，会导致硬件状态混乱（如寄存器写入一半被打断），因此必须先禁用 FIQ/IRQ。

### 4.启用 SMP 支持（对称多处理器）

```asm
/* Enable ACTLR.SMP bit */
mrc	p15, 0, r0, c1, c0, 1  // 读取 ACTLR 寄存器（辅助控制寄存器，控制 CPU 核心特性）
orr	r0, r0, #(1 << 6)      // 置位 bit6（SMP 位）——启用多核支持（让多个 CPU 核心能协同工作）
mcr	p15, 0, r0, c1, c0, 1  // 写回 ACTLR，生效 SMP 配置
```

- **背景**：若芯片是多核（如 Cortex-A9 双核），需启用 SMP 位才能让从核（Slave Core）被主核（Master Core）唤醒，否则只能单核运行。
- **ACTLR 寄存器**：ARMv7 用于控制核心级特性（如 SMP、缓存策略）的关键寄存器。

### 5.向量表重定位

可选，除 OMAP4 SPL 外。

```asm
#if !(defined(CONFIG_OMAP44XX) && defined(CONFIG_SPL_BUILD))
	/* Set V=0 in CP15 SCTLR register - for VBAR to point to vector */
	mrc	p15, 0, r0, c1, c0, 0	@ 读取 SCTLR 寄存器（系统控制寄存器，控制 MMU、缓存、向量表模式）
	bic	r0, #CR_V			   @ 清除 V 位（bit13）——禁用“高向量表”模式，让 VBAR 寄存器生效
	mcr	p15, 0, r0, c1, c0, 0	@ 写回 SCTLR

	/* Set vector address in CP15 VBAR register */
	ldr	r0, =_start            	@ 加载 U-Boot 向量表的起始地址（_start 是链接脚本指定的向量表入口）
	mcr	p15, 0, r0, c12, c0, 0	@ 将向量表地址写入 VBAR 寄存器（向量表基地址寄存器）
#endif
```

- **向量表的作用**：
  向量表是 ARM 处理异常（如中断、复位、未定义指令）的 “地址表”，CPU 发生异常时会自动跳转到向量表中对应的地址执行处理函数。
- **为什么重定位**：
  芯片上电时，向量表默认在 `0x00000000`（ROM 地址），但 U-Boot 运行后需要使用自己的向量表（处理 U-Boot 阶段的异常），因此需将向量表基地址重定位到 U-Boot 的 `_start` 地址（通常在 DDR 或 SRAM 中）。

###  6.启用异步外部中止（避免硬件错误被忽略）

```asm
/* Enable Asynchronous external abort after vectors setup */
mrs	r0, cpsr
bic	r0, r0, #0x100		@ 清除 CPSR 的 A 位（bit8）——启用异步外部中止
msr	cpsr_x,r0            @写回 CPSR 的扩展部分（cpsr_x 是 CPSR 的别名，针对 ARMv7）
```

- **异步外部中止**：
  指硬件发生的不可预测错误（如访问无效内存地址、外设故障），启用后 CPU 会捕获这些错误并跳转到向量表的 “中止处理函数”，避免程序在错误状态下继续运行（如死机）。

### 7.底层硬件初始化

可选，跳过需配置宏。

```asm
/* the mask ROM code should have PLL and others stable */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT  @若未配置“跳过底层初始化”
	bl	cpu_init_cp15         	@1. 初始化 CP15 寄存器（缓存、MMU、TLB 等）
	#ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY @若未配置“仅跳过部分底层初始化”
		bl	cpu_init_crit       @2. 初始化关键硬件（板级时钟、内存时序等）
	#endif
#endif

bl	_main                 // 跳转到 C 语言主函数 _main（U-Boot 主体逻辑入口）
```

- **`CONFIG_SKIP_LOWLEVEL_INIT`**
  若某块开发板的 ROM 已完成大部分底层初始化（如 PLL 时钟、内存），可配置该宏跳过 U-Boot 的重复初始化，加快启动速度。
- **核心跳转 `bl _main`**：这是汇编阶段到 C 语言阶段的 “桥梁”，`_main` 函数位于 `common/main.c`，负责后续 U-Boot 主体逻辑（如环境变量加载、驱动初始化、命令行交互）。

## 3.关键辅助函数解析

### 1. CPU 运行时缓存配置

```asm
ENTRY(c_runtime_cpu_setup)
/* If I-cache is enabled invalidate it */
#ifndef CONFIG_SYS_ICACHE_OFF  // 若未禁用指令缓存（I-Cache）
	mcr	p15, 0, r0, c7, c5, 0	    @  invalidate icache（ invalidate I-Cache，清除指令缓存）
	mcr     p15, 0, r0, c7, c10, 4	@ DSB（数据同步屏障，确保缓存操作完成）
	mcr     p15, 0, r0, c7, c5, 4	@ ISB（指令同步屏障，确保后续指令从新缓存中读取）
#endif
bx	lr                  // 返回调用者
ENDPROC(c_runtime_cpu_setup)
```

- **作用**：在 C 语言代码运行前，确保指令缓存（I-Cache）处于有效且干净的状态。I-Cache 用于缓存已执行的指令，启用后可提升 CPU 执行效率，但初始化阶段需先清除旧缓存数据（避免脏数据干扰）。
- **`DSB/ISB`**：ARM 内存屏障指令，确保缓存操作（如 invalidate）完成后再执行后续指令，避免硬件乱序执行导致的错误。

### 2.CP15 寄存器初始化（核心中的核心）

`CP15` 是 ARMv7 的 **系统控制协处理器**，负责管理缓存、MMU、TLB、时钟等核心硬件，`cpu_init_cp15` 是对其的基础配置，可拆解为 4 个关键步骤：

#### 1.清除 TLB 和缓存

```asm
mov	r0, #0				   @ 准备 0 值用于 MCR 指令
mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs（清除所有 TLB 条目，TLB 是 MMU 的页表缓存）
mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache（清除 I-Cache）
mcr	p15, 0, r0, c7, c5, 6	@ invalidate BP array（清除分支预测缓存，避免错误分支预测）
mcr     p15, 0, r0, c7, c10, 4	@ DSB（确保缓存操作完成）
mcr     p15, 0, r0, c7, c5, 4	@ ISB（确保后续指令从新缓存读取）
```

- **为什么清除**：芯片上电后，TLB、缓存中可能残留 ROM 或测试阶段的脏数据，若不清除，后续操作会读取错误数据（如执行旧指令、访问错误内存地址）。

#### 2.禁用 MMU 和配置缓存

```asm
mrc	p15, 0, r0, c1, c0, 0  // 读取 SCTLR 寄存器（系统控制寄存器）
bic	r0, r0, #0x00002000	@ 清除 bit13（V 位，禁用高向量表）
bic	r0, r0, #0x00000007	@ 清除 bit0-bit2（CAM 位，禁用 MMU、数据缓存、指令缓存）
#if 0 /* 注释：因固件解压时可能有非对齐访问，暂不启用对齐检查 */
orr	r0, r0, #0x00000002	@ 置位 bit1（A 位，启用对齐检查）
#endif
orr	r0, r0, #0x00000800	@ 置位 bit11（Z 位，启用分支目标缓冲器 BTB，优化分支指令执行）
#ifdef CONFIG_SYS_ICACHE_OFF  // 若禁用 I-Cache
bic	r0, r0, #0x00001000	@ 清除 bit12（I 位，禁用 I-Cache）
#else
orr	r0, r0, #0x00001000	@ 置位 bit12（I 位，启用 I-Cache）
#endif
mcr	p15, 0, r0, c1, c0, 0	@ 写回 SCTLR，生效配置
```

- **关键逻辑**：初始化阶段（汇编阶段）暂不启用 MMU（内存管理单元）和数据缓存（D-Cache），仅根据配置决定是否启用 I-Cache。因为 MMU 启用需要页表支持，而页表配置依赖 C 语言代码（`_main` 中处理）。

#### 3.处理 ARM 芯片勘误（Errata）

代码中大量 `#ifdef CONFIG_ARM_ERRATA_XXXXXX` 块（如 `CONFIG_ARM_ERRATA_716044`、`CONFIG_ARM_ERRATA_794072`），作用是：

- **芯片勘误**：ARM 芯片量产时可能存在硬件设计缺陷（Errata），需通过配置特定寄存器来规避（如调整缓存策略、禁用某功能）。

- 实现逻辑：通过`mrc`读取诊断寄存器，置位特定位（启用规避措施），再通过`mcr`写回。例如：

  ```asm
  #ifdef CONFIG_ARM_ERRATA_716044
  mrc    p15, 0, r0, c1, c0, 0    @ 读取 SCTLR
  orr    r0, r0, #1 << 11    @ 置位 bit11，规避勘误 716044
  mcr    p15, 0, r0, c1, c0, 0    @ 写回 SCTLR
  #endif
  ```

#### 4.返回调用者

```asm
mov	r5, lr			@ 保存返回地址（lr 寄存器，调用者的下一条指令地址）
/* ... 中间省略 CPU 版本检查和勘误处理 ... */
mov	pc, r5			@ 跳回调用者（pc = lr，回到之前的执行流程）
ENDPROC(cpu_init_cp15)
```

### 3. 板级关键硬件初始化

```asm
cpu_init_crit
ENTRY(cpu_init_crit)
/* Jump to board specific initialization... */
b	lowlevel_init		@ 跳转到板级底层初始化函数
ENDPROC(cpu_init_crit)
```

- **`lowlevel_init` 的作用**：这是板级专属的底层初始化函数（位于`board/[厂商]/[芯片]/lowlevel_init.S`），负责初始化 “芯片级关键硬件”，如：
  - 配置 PLL 时钟（提升 CPU 和外设时钟频率，从 ROM 默认的低频率到实际工作频率）；
  - 初始化 DDR 内存时序（确保 DDR 能稳定工作）；
  - 配置电源管理芯片（给 CPU、DDR 提供稳定电压）。
- **为什么放在板级代码**：不同开发板的时钟、内存、电源配置差异极大（如同一芯片可能搭配不同频率的 DDR），因此 U-Boot 将这部分代码放在板级目录，由开发者根据硬件定制。

## 4.弱符号函数

代码末尾定义了两个弱符号函数，体现 U-Boot 的 “通用代码 + 板级定制” 设计思路：

```asm
/* save_boot_params：弱符号，默认直接返回 */
ENTRY(save_boot_params)
b	save_boot_params_ret		@ 跳回返回点，无额外操作
ENDPROC(save_boot_params)
.weak	save_boot_params  // 标记为弱符号，允许板级代码覆盖

#ifdef CONFIG_ARMV7_LPAE
/* switch_to_hypervisor：弱符号，默认直接返回 */
ENTRY(switch_to_hypervisor)
b	switch_to_hypervisor_ret
ENDPROC(switch_to_hypervisor)
.weak	switch_to_hypervisor
#endif
```

- **弱符号的价值**：若某块开发板需要特殊处理（如 save_boot_params 需保存 ROM 传递的硬件参数，或 switch_to_hypervisor 需执行定制化虚拟化初始化），可在板级代码中重写同名函数，覆盖弱符号实现，无需修改通用汇编代码。

## 5.代码的核心作用与地位

这段汇编代码是 U-Boot 启动的 **“地基”**，完成了从 “芯片上电” 到 “C 语言运行” 的所有底层准备，核心贡献包括：

1. **CPU 模式与安全配置**：进入 SVC 特权模式，禁用中断，避免初始化被干扰；
2. **协处理器初始化**：配置 CP15 寄存器，清除缓存 / TLB，启用必要的核心特性（如 I-Cache、SMP）；
3. **硬件勘误规避**：处理 ARM 芯片的硬件缺陷，确保运行稳定性；
4. **板级初始化衔接**：跳转到 `lowlevel_init` 完成板级关键硬件（时钟、DDR）配置；
5. **C 语言入口跳转**：最终通过 `bl _main` 进入 U-Boot 主体逻辑（C 语言代码）。

------

# 参考链接

[ARM指令集详解(超详细！带实例!）_arm指令编程手册-CSDN博客](https://blog.csdn.net/mickey35/article/details/82011449)
