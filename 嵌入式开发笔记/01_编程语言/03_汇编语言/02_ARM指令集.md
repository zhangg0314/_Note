# 数据处理指令

## 1.数据搬移指令

```asm
MOV{条件}{S}  <dest>, <op 1> @ dest = op_1
MVN{条件}{S}  <dest>, <op 1> @ dest = !op_1

@{条件}：
条件码后缀
@{S}：
影响条件位N,Z,V,C寄存器的后缀
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
## 2.数据运算指令

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

## 3.位运算

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

## 4.带S的数据运算指令

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

# 跳转指令

## 1.程序跳转本质

实现程序的跳转，本质就是修改PC寄存器。

## 2.跳转方式

### 1.直接修改PC的值

```asm
MOV PC,#0x18
@但需要我们自己去计算绝对地址，很麻烦。
```

### 2.不带返回的跳转指令

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

### 3.带返回的跳转指令

```asm
BL FUNC
@本质是将PC寄存器的值修改成跳转标号下第一条指令的地址，同时将跳转指令下一条指令的地址存储到当前模式下的LR
```

### 4.带条件的跳转，但不带返回

```asm
CMP R1,R2
BEQ FUNC 
@CPU先看Z是否置1，如果是，则跳转FUNC
@EQ为条件码
@不带返回
```

# 移位指令

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

# ARM中的栈

## 1.概念

SP寄存器的内容存放的就是栈顶指针的地址，而栈的本质就是一段内存即RAM，用于程序运行时保存一些临时数据。如局部变量，函数参数，返回值等。

## 2.栈的分类

针对于压栈时的特点有如下分类：

- 增栈--存数据时，SP往高地址方向移动。

- 减栈--存数据时，SP往低地址方向移动。

- 满栈：栈指针指向最后一次压入到栈中的数据，压栈时需要先移动栈指针到相邻位置再压栈。

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

# 伪指令

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

# 伪操作

## 1.特性

```asm
@伪操作一般以'.'开头
```

## 2.全局局部属性

```asm
@1.声明全局标识
	.global symbol_name
	.globl 	symbol_name
	#将一个如MAIN,STOP这样的符号定义为全局符号，使得链接器能够全局识别它，即一个程序文件中定义的符号能够被所有其他程序文件可见。

@2.声明局部标识
	.local symbol_name  
	#将一个符号定义为局部符号，使得此符号不能够被其他程序文件可见。
```

## 3.属性弱化

```asm
@1.弱化属性
	.weak symbol_name
	#在汇编程序中，符号的默认属性为强（strong），.weak伪操作则用于设置符号的属性为弱（weak），如果此符号之前没有定义过，则同时创建此符号并定义其属性为weak。
```

如果符号的属性为weak，那么它无需定义具体的内容。在链接的过程中，另外一个属性为strong的同名符号可以将此weak符号的内容强制覆盖。利用此特性，.weak伪操作常用于预先预留一个空符号，使得其能够通过汇编器语法检查，但是在后续的程序中定义符号的真正实体，并且在链接阶段将空符号覆盖并链接。

## 4.类型指定

```asm
.type  symbol_name , type description

#.type伪操作用于定义符号的类型。
#譬如“.type symbol,@function”即将名为symbol的符号定义为一个函数（function）。
```

## 5.地址对齐

```asm
	.align n
	#让下一条语句地址对齐到地址为2^n的位置,实质就是将当前PC地址推进到“2的integer次方个字节”对齐的位置。
	
	.balign n
	#伪操作用于将当前PC地址推进到“n个字节”对齐的位置。
	
	.zero n
	#zero伪操作将从当前PC地址处开始分配n个字节空间并且用0值填充。
```

## 6.空间分配

### 1.申请单个字节

```asm
.byte
.byte expression [, expression]*
.byte 0xFF
#.byte伪操作将从当前PC地址处开始分配若干个字节（byte）的空间，每个字节填充的值由分号分隔开的expression指定。
```

### 2.申请双字节

```asm
.2byte
.2byte expression [, expression]*
#.2byte伪操作将从当前PC地址处开始分配若干个双字节（2 bytes）的空间，每个双字节填充的值由分号分隔开的expression指定。空间分配的地址可以与双字节非对齐。
```

### 3.申请四字节

```asm
.4byte
.4byte expression [, expression]*
#.4byte伪操作将从当前PC地址处开始分配若干个四字节（4 bytes）的空间，每个四字节填充的值由分号分隔开的expression指定。空间分配的地址可以与四字节非对齐。
```

### 4.申请八字节

```asm
.8byte
.8byte expression [, expression]*
#.8byte伪操作将从当前PC地址处开始分配若干个八字节（8 bytes）的空间，每个八字节填充的值由分号分隔开的expression指定。空间分配的地址可以与八字节非对齐。
```

### 5.申请半个字

```asm
.half
.half expression [, expression]*

#.half伪操作将从当前PC地址处开始分配若干个半字（half-word）的空间，每个半字填充的值由分号分隔开的expression指定。空间分配的地址一定与半字对齐（half-word aligned）。
```

### 6.申请一个字

```asm
.word
.word expression [, expression]*
#.word伪操作将从当前PC地址处开始分配若干个字（word）的空间，每个字填充的值由分号分隔开的expression指定。空间分配的地址一定与字对齐（word aligned）。


@eg:
MOV R1,#1
.word 0xFFFFFFFF     @在当前地址申请一个字的空间并将其初始化0xFFFFFFFF
MOV R2,#2
```

### 7.申请双字

```asm
.dword
.dword expression [, expression]*
#.dword伪操作将从当前PC地址处开始分配若干个双字（double-word）的空间，每个双字填充的值由分号分隔开的expression指定。空间分配的地址一定与双字对齐（double-word aligned）。
```

### 8.申请字符串

```asm
.string
.string “string”
#.string伪操作将从当前PC地址处开始分配若干个字节空间用于存放“string”字符串。字节的个数取决于字符串的长度。
```

### 9.申请单精度浮点

```asm

```

### 10.申请双精度浮点

```asm






.assicz
#.assicz汇编指令用来向当前段添加字符串。字符串是一串字节序列表示。.string和.asciz会在字符串末尾额外添加一个0值的字节，即NULL结尾的字符串。

.assic
#.assic汇编指令用来向当前段添加字符串

.float
.float 或者 .double expression [, expression]*

#.float伪操作将从当前PC地址处开始分配若干个单精度浮点数（32位）的空间，每个单精度浮点数填充的值由分号分隔开的expression指定。空间分配的地址一定与32位对齐。
#.double伪操作将从当前PC地址处开始分配若干个双精度浮点数（64位）的空间，每个双精度浮点数填充的值由分号分隔开的expression指定。空间分配的地址一定与64位对齐。

.space n,#0x12        @申请任意字节n，并将其初始化

.comm
.comm或者.common name, length
#.comm和.common伪操作用于声明一个名为name的未初始化存储区间，区间大小为length个字节。
```

## 7.宏声明

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



@9.
MOV R1,#1


MOV R2,#2


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

------

# 参考链接

[ARM指令集详解(超详细！带实例!）_arm指令编程手册-CSDN博客](https://blog.csdn.net/mickey35/article/details/82011449)

[ARMv7-A 那些事 - 5.CP15协处理器 - 知乎](https://zhuanlan.zhihu.com/p/663981666)
