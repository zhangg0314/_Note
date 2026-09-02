

# 基本概述

1. **含义**

   make 是一个命令，它通过读取一个名为 Makefile（或   makefile  ）的配置文件来自动执行一系列的构建任务。make，说简单点，就是制作，如果Makefile中定义的需要制作的目标在当前目录已有且是最新的，则make不会执行，目标就用当前目录的，没有就根据制作规则语句进行制作。

2. **作用**
   1.一旦写好makefile文件，只需一个make命令，整个工程完全自动编译，极大提高了软件开发的效率。

   2.提升编译效率：不是一步编译时，第二次make时，只编译修改了的文件。其是利用文件时间戳来确定文件是否被修改过了。

   3.make 的主要用途是自动化构建过程，特别是编译和链接程序。

# 基本语法

## 1.语法格式

```makefile
目标（target）... : 依赖（prerequisites） ...
	命令（command）
```

## 		2.具体解释

### 1.目标

目标是我们要生成的东西，省略文件的路径时，默认为生成到当前路径下。

```makefile
start.o : start.S
	arm-linux-gcc -c start.S
#会在当前目录下生成start.o这个文件，会新多出一个文件

lowlevel_init.o : lowlevel_init.S
	arm-linux-gcc -c lowlevel_init.S
	
#如果直接输入make命令。只会去生成第一个start.o目标，而不会去生成其他的目标。
```

目标文件可以是我们所需的最终文件或者可执行文件，即能正在生成出来的一个文件；也可以仅仅是为了执行某些命令而实现某个功能，并不生成文件。

```makefile
.PHONY#虚假的意思
all: start.o lowlevel_init.o

start.o : start.S
	arm-linux-gcc -c start.S
#会在当前目录下生成start.o这个文件，会新多出一个文件

lowlevel_init.o : lowlevel_init.S
	arm-linux-gcc -c lowlevel_init.S
	
#如果直接输入make命令。只会去生成第一个all目标，就会去寻找依赖，如果当前目录下没有依赖，就会去寻找生成依赖的makefile语句。
#由于all是伪目标，因此不会去生成一个名为all的文件。
```

### 2.依赖

要生成目标target所需要的文件，makefile默认会在当前目录下寻找所需的依赖文件，如果没有则找有无生成该依赖的makefile语句，此时依赖就是另外一条语句的目标。

### 3.命令

任意shell命令，不一定是gcc命令，**@**作用是不显示命令再显示结果而是直接显示执行命令的结果。

------

# 		语句解析规则

## 1.只生成一个目标

只输入一个`make`，在makefile中默认只找第一条目标。故执行有多条语句的makefile时，应把生成最终目标的语句放在最前面，因为make默认只识别makefile的第一条语句，第一条语句在其找依赖文件时，默认会**在当前目录**下找，如果有该名称的文件，则直接用该文件，如果没有，则makefile中找是否有该依赖文件作为目标文件存在于另外一条语句中，有该语句，则先执行完该语句。就这样带动一连串的make语句被执行。

## 2.生成指定目标

```bash
make  目标名
#则寻找生成该目标的语句进行执行。
```

考虑一个场景：

```makefile
.PHONY
all: start.o lowlevel_init.o
	arm-linux-ld -Tmyboot.lds -o myboot start.o lowlevel_init.o
	arm-linux0objcopy -o binary myboot myboot.bin
	sudo dd if=myboot.bin of=/dev/sdb seek=1
start.o : start.S
	arm-linux-gcc -c start.S
#会在当前目录下生成start.o这个文件，会新多出一个文件

lowlevel_init.o : lowlevel_init.S
	arm-linux-gcc -c lowlevel_init.S
	
#如果直接输入make命令。只会去生成第一个all目标，就会去寻找依赖，如果当前目录下没有依赖，就会去寻找生成依赖的makefile语句。
#由于all是伪目标，因此不会去生成一个名为all的文件。
```

在每次`make`的时候，makefile都会顺序的往下执行，如果此时没插入SD卡，或者插入SD卡系统识别的名字不是`/dev/sdb`.那么当执行到下面语句时会报错，这是我们不希望看到的，因此就需要对该语句进行选择性执行，但又得保留makefile提供给我们得便利，因此可以再定义一个目标或者伪目标，就不会一直被执行，除法`make`时主动执行。

```bash
sudo dd if=myboot.bin of=/dev/sdb seek=1
```

改进如下：

```makefile
.PHONY
all: start.o lowlevel_init.o
	arm-linux-ld -Tmyboot.lds -o myboot start.o lowlevel_init.o
	arm-linux0objcopy -o binary myboot myboot.bin
start.o : start.S
	arm-linux-gcc -c start.S
#会在当前目录下生成start.o这个文件，会新多出一个文件

lowlevel_init.o : lowlevel_init.S
	arm-linux-gcc -c lowlevel_init.S

.PHONY
mksd:
	sudo dd if=myboot.bin of=/dev/sdb seek=1
```

```bash
make mksd
```

## 3.伪目标的优点

1.解决默认只执行第一条语句的局限，介绍见上文。

2.**指定执行的语句**中的目标名（可以是伪目标）与当前目录下文件名重名的冲突，此外新生成的文件会覆盖当前目录下重名的那个文件，以.PHONY做目标，会找并执行第一条伪目标语句。

## 4.伪目标局限及改进

考虑如下场景：

```makefile
.PHONY
all: start.o lowlevel_init.o
	arm-linux-ld -Tmyboot.lds -o myboot start.o lowlevel_init.o
	arm-linux0objcopy -o binary myboot myboot.bin
start.o : start.S
	arm-linux-gcc -c start.S
#会在当前目录下生成start.o这个文件，会新多出一个文件

lowlevel_init.o : lowlevel_init.S
	arm-linux-gcc -c lowlevel_init.S

.PHONY
mksd:
	sudo dd if=myboot.bin of=/dev/sdb seek=1
```

由前面可以知道，伪目标all并不会正在去生成all文件，那就相当于没有all文件，而make语句执行与否是通过对比目标和依赖的最后修改时间来进行的，但不存在all文件，因此执行伪目标的时候，会一直认为all的依赖文件是最新的，因此每次`make`时都会执行all伪目标的那几条语句，这也是我们不希望看到的，因此可以改进我们的伪目标的定义。

```makefile
.PHONY
all: myboot

myboot: start.o lowlevel_init.o
	arm-linux-ld -Tmyboot.lds -o myboot start.o lowlevel_init.o
	arm-linux0objcopy -o binary myboot myboot.bin
#目标之所以选择myboot，是因为myboot是直接由 start.o lowlevel_init.o生成而来的，只要判断myboot的依赖文件未更新，就不会执行生成myboot的命令，myboot就不会更新，则不会执行任何语句了。	
	
start.o : start.S
	arm-linux-gcc -c start.S
#会在当前目录下生成start.o这个文件，会新多出一个文件

lowlevel_init.o : lowlevel_init.S
	arm-linux-gcc -c lowlevel_init.S

.PHONY
mksd:
	sudo dd if=myboot.bin of=/dev/sdb seek=1
```



------

# 		自动化变量

## 1.产生背景

倘若有成千上万的文件需要编译，不可能为每个文件编写一条makefile语句，因此为解决这个问题引入了自动化变量。自动化变量主要针对文件名，自动指变量的值会根据修改自动调整。

## 2.通配符

`%.o`表示当前目录下所有的`.o`文件，`%.s`表示当前目录下所有的`.s`文件。

```makefile
start.o : start.S
	arm-linux-gcc -c start.S
#会在当前目录下生成start.o这个文件，会新多出一个文件

lowlevel_init.o : lowlevel_init.S
	arm-linux-gcc -c lowlevel_init.S
============================================================
%.o : %.s
	arm-linux-gcc -c %<

```

?????????

**能使用%.c ：%.o这样的通配符的前提是：**

在通配符使用语句前已经出现过了%.c的范围是哪些.c文件，不然%可以匹配任意，那么有无穷多个目标了，此外通配符并不匹配路径，所以带有路径的文件通配，要自己加上路径。

## 3.表示依赖文件

- $+所有依赖文件，可重复

- `$^ `
  所有依赖文件的集合，文件不重复。      

- `$<`
  表示所有依赖文件集合中的第一个依赖文件，所谓的第一个是按生成时间排序的第一个。

- $?:所有时间戳比目标文件晚的依赖文件，即上一次修改时间比目标文件生成时间晚的文件，用于打印第二次编译时哪些依赖文件被修改了。

## 4.表示目标文件

- $*：不包含后缀 ，用于打印，对于本身没后缀的无效，对于可执行文件a.txt，他还是没后缀，.txt也算文件名不算后缀名，故打印无后缀的只能用$@  
- `$@`
  所有目标集合，集合里的每个目标文件都是完整的文件名字，有后缀名的也会包含后缀名，可以用于打印和编译 。       


 	比如提高编程速度，每次只要写一句`gcc`命令再不断复制粘贴带有自动变量的相同的语句即可。makefile隐含变量，针对shell命令，可以赋值，不赋值则用默认值.

#### 3.命令（有默认值）

- CC：默认值--cc 赋值交叉编译 CC = arm-linux-gcc

- CPP：默认值--$(CC) -E预编译

- RM：默认值rm -f


#### 3.命令的选项（无默认值）

- CFLAGS= -c -g -Wall 

- CPPFLAGS

- CXXFLAGS

- LDFLAGS

### 		1.创建变量的目的

用来代替一个**文本字符串**，该字符串可以是：

1.系列文件的名字

2.传递给编译器的参数

3.需要运行的程序

4.需要查找源代码的目录

5.需要输出信息的目录

6.etc

最终就是便于维护和修改makefile文件，改一处，实现多次改。

### 		2.自定义变量

1. 声明时就要赋值，赋值多个字符串时，用空格隔开。
2. 取值方式：$(x)或${x}==取x的值，$$表示打印真实的$符号。
3. SRC = $(shell pwd)/obj ：shell命令执行结果作为值给变量赋值，但$(shell pwd）会被立马执行，其结果为make执行的当前工作目录，当SRC传递其他目录下的文件时，传递的并不是$(shell pwd),而是make工作目录。

### 		3.变量赋值

- =一般赋值,多个条语句对同一变量赋值，最终结结果为最后一条语句的值
- ?=如果前面已经给该变量赋值了，则此次赋值失效，否则执行此次赋值 

- :=如果前面有值了则覆盖掉原来的值，与？=相反

- +=追加赋值

## 	4.makefile的条件判断

1. ifeq（a,b）：a,b是否相等，if与（）间要隔开！！！

2. ifneq：是否不相等

3. ifdef  ARCH：只有ARCH**定义过且赋过值**才算定义过，无论是在文件中定义还是在传参时定义，规则都一样

4. ifndef   endif


# 	常用标准函数

## 1.标准格式

```makefile
$(<函数名> <参数1,参数2>)
${<函数名> <参数1,参数2>}
```

## 2.字符串与路径解析

一个文件名正常应该是带绝对路径的文件名，缺省绝对路径只有文件名的化默认都是当前目录下的文件。当文件名（无论有路径）赋值给变量时，其特性就不是文件了，就是一个字符串，赋值的时候缺省路径那么该字符串就没有路径，变量就是变量，别指望变量会记住"这是个文件、它在哪个目录"，**规则里的是文件，变量里的是字符串；裸文件名 = 默认当前目录，这个默认值在赋值时不会自动补进字符串里**

### 1.核心语义

GNU Make 的一条核心语义：**Makefile 里的"文件名"只是字符串，路径解析发生在"使用"时而不是"赋值"时。**

|        时机        |                          发生了什么                          |
| :----------------: | :----------------------------------------------------------: |
|       赋值时       |                  纯字符串操作，不碰文件系统                  |
|  作为依赖/目标时   | make 才去文件系统找文件，相对路径按工作目录（或 `VPATH`/`vpath`）解析 |
| 在 recipe 命令行里 |       传给 shell 执行，由 shell 按当前目录解析相对路径       |

```makefile
SRCS = main.c          # 只是字符串 "main.c"
main: $(SRCS)
	gcc $(SRCS) -o main   # 这里才真的在当前目录找 main.c
```

### 2.实际指导意义

1. **路径安全**
   想让路径可靠，显式加工：`$(CURDIR)/...`、`$(abspath ...)`、`$(realpath ...)`，或写成 `src/main.c` 这类相对路径，而不是裸文件名。
2. **不要假设**
   `OBJS = main.o foo.o` 只是名字，最终在哪生成/查找取决于规则目标和 make 的运行目录。
3. **一句话总结**：**规则里的是文件，变量里的是字符串；裸文件名 = 默认当前目录，这个默认值在赋值时不会自动补进字符串里。**

---

## 3.常用函数大全

### 1.字符串处理类

|           函数            |                  作用                  |                     示例                     |
| :-----------------------: | :------------------------------------: | :------------------------------------------: |
|  `$(subst from,to,text)`  |   把 text 中所有 `from` 替换为 `to`    |      `$(subst .c,.o,main.c)` → `main.o`      |
|  `$(patsubst p,r,text)`   |         按模式替换（`%` 通配）         |   `$(patsubst %.c,%.o,main.c)` → `main.o`    |
|      `$(strip str)`       |        去首尾空白并压缩连续空格        |        `$(strip "  a   b ")` → `a b`         |
|  `$(findstring find,in)`  | 在 in 里找 find，找到返回 find，否则空 |         `$(findstring a,abc)` → `a`          |
|   `$(filter p...,text)`   |           保留匹配模式的单词           |   `$(filter %.c,main.c main.o)` → `main.c`   |
| `$(filter-out p...,text)` |           去掉匹配模式的单词           | `$(filter-out %.o,main.c main.o)` → `main.c` |
|      `$(sort list)`       |           按字典序排序并去重           |         `$(sort b a c a)` → `a b c`          |
|     `$(word n,text)`      |       取第 n 个单词（从 1 开始）       |           `$(word 2,a b c)` → `b`            |
|  `$(wordlist s,e,text)`   |          取第 s 到第 e 个单词          |      `$(wordlist 2,3,a b c d)` → `b c`       |
|      `$(words text)`      |              统计单词个数              |            `$(words a b c)` → `3`            |
|    `$(firstword text)`    |              取第一个单词              |          `$(firstword a b c)` → `a`          |
|    `$(lastword text)`     |             取最后一个单词             |          `$(lastword a b c)` → `c`           |

**快捷写法**：`$(var:%.c=%.o)` ≡ `$(patsubst %.c,%.o,$(var))`；`$(var:.c=.o)` ≡ `$(subst .c,.o,$(var))`。最常用：

```makefile
OBJS = $(SRCS:.c=.o)        # 或 $(SRCS:%.c=%.o)
```

### 2.文件名处理类

|           函数            |                  作用                  |                      示例                       |
| :-----------------------: | :------------------------------------: | :---------------------------------------------: |
|     `$(dir names...)`     |        取目录部分（含末尾 `/`）        |          `$(dir src/main.c)` → `src/`           |
|   `$(notdir names...)`    |              取文件名部分              |        `$(notdir src/main.c)` → `main.c`        |
|   `$(suffix names...)`    |                 取后缀                 |            `$(suffix main.c)` → `.c`            |
|  `$(basename names...)`   |                去掉后缀                |          `$(basename main.c)` → `main`          |
| `$(addprefix p,names...)` |                 加前缀                 | `$(addprefix obj/,a.o b.o)` → `obj/a.o obj/b.o` |
| `$(addsuffix s,names...)` |                 加后缀                 |        `$(addsuffix .o,a b)` → `a.o b.o`        |
|      `$(join l1,l2)`      |           两个列表按位置拼接           |           `$(join a b,1 2)` → `a1 b2`           |
|   `$(wildcard pattern)`   |     展开通配符，返回真实存在的文件     |               见下方 U-Boot 示例                |
|  `$(realpath names...)`   |  绝对路径（**要求文件存在**，否则空）  |            `$(realpath ./Makefile)`             |
|   `$(abspath names...)`   | 绝对路径（**不要求存在**，只做规范化） |             `$(abspath ./nonexist)`             |

```makefile
# 判断配置文件是否存在
ifneq ($(wildcard include/config/auto.conf),)
    ...
endif

# 收集 .cmd 依赖文件：对每个 target 取目录 + 取文件名
cmd_files := $(wildcard .*.cmd $(foreach f,$(targets),$(dir $(f)).$(notdir $(f)).cmd))

# wildcard 与 if 配合：文件存在返回 y，否则 n
HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(srctree)/board/$(VENDOR)/common/Makefile),y,n)
```

### 3.函数举例

```shell
$(wildcard PATTERN)
	#功能：列出当前目录下所有符合模式PATTERN格式的文件名,比如*.c列出当前目录下所有.c文件
	#返回值：空格分割的，存在当前目录下的所有符合模式PATTERN格式的文件名,PATTERN是文件名，不是字串！！！	
	#PATTERN：可以使用shell下的可识别的所有通配符
#作用：
	SRC = add.c sub.c test.c xxx.c.....www.c
	#此时SRC仅仅只是一串字符，不会说有默认当前目录下的这些文件这种说法
	#取代：SRC = $(wildcard *.c),大大简洁了代码！！
$(pastsubst <pattern>,<repalcement>,<text>)
	#它用于将一个字符串中符合特定模式的子串替换为另一个字符串，不符合的字串就不换
	#text:要进行替换的对象，若有多个则用空格隔开，text是字串
	#pattern：替换对象满足的文件名字格式，pattern也是字串。
	#replacement：替换成什么格式的文件名，replacement也是字串。
	#返回值是替换后的文件名，包含没有被替换的，比如，text为1.c，2.cpp，pattern为*.c，则返回值为1.o,2.cpp
```

### 4.条件类函数

|           函数           |                        作用                        |
| :----------------------: | :------------------------------------------------: |
| `$(if cond,then[,else])` | cond 非空则展开 then，否则展开 else（else 可省略） |
|     `$(or a,b,...)`      |              从左到右返回第一个非空值              |
|     `$(and a,b,...)`     |          全部非空才返回最后一个值，否则空          |

注意：这些是**函数**（表达式内求值），与 `ifeq`/`ifdef` 等**条件指令**（控制整个 Makefile 分支）不同。

```makefile
HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(srctree)/board/$(VENDOR)/common/Makefile),y,n)
CFLAGS = $(if $(DEBUG),-g,-O2)
```

### 5.循环函数

`$(foreach var,list,text)` —— 把 list 里的每个单词依次赋给 `var`，展开 `text`，结果按空格拼接：

```makefile
# U-Boot 中：遍历所有子目录，只保留带 Makefile 的目录
clean-dirs := $(foreach f,$(u-boot-alldirs),$(if $(wildcard $(srctree)/$f/Makefile),$f))
```

注意：循环体里要修改别的变量需配合 `$(eval)` 才能持久生效。

### 6.执行 shell 命令

`$(shell command)` —— 执行 shell 命令并把**标准输出**作为结果（末尾换行被去掉）：

```makefile
ARCH := $(shell uname -m)
DATE := $(shell date +%Y%m%d)
SRCS := $(shell find src -name "*.c")
```

⚠️ 注意：

1. 每次 `$(shell)` 都起一个子进程，**不能在 `$(foreach)` 大循环里滥用**。
2. 命令输出为空时变量是空字符串，`ifdef` 判断会失效，常配合 `$(strip)` 或 `$(if)` 使用。

### 7.控制类函数（调试/报错）

|       函数        |              作用              |
| :---------------: | :----------------------------: |
|  `$(info text)`   | 打印信息，不报错，**总是求值** |
| `$(warning text)` |    打印警告，make 继续执行     |
|  `$(error text)`  |  打印错误并**立即终止** make   |

```makefile
ifeq ($(CONFIG_CPU),)
$(error CONFIG_CPU is not set! Please configure first)
endif
```

调试技巧：`$(info VAR=$(VAR))` 或 `$(warning ...)` 是排查变量展开问题最常用的手段。

### 8.其他常用函数

|             函数              |                             作用                             |                             说明                             |
| :---------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| `$(call name,arg1,arg2,...)`  |                 调用自定义"函数"（带参变量）                 |   `define name` ... `endef`，内部用 `$(1)` `$(2)` 引用参数   |
|        `$(eval text)`         |            把 text 当作 Makefile 内容动态展开解析            |                配合 `$(foreach)` 动态生成规则                |
|        `$(value var)`         |                   取变量的**原始未展开**值                   |                        对递归变量有用                        |
|        `$(origin var)`        | 返回变量来源（`command line`/`environment`/`file`/`undefined` 等） | U-Boot 用 `ifeq ("$(origin V)", "command line")` 判断 V 是否来自命令行 |
|        `$(flavor var)`        |                 返回 `recursive` 或 `simple`                 |               判断变量是 `=` 还是 `:=` 定义的                |
|  `$(file op filename,text)`   |            读写文件（`>` 写、`>>` 追加、`<` 读）             |                         较新版本才有                         |
| `$(intcmp ...)`、`$(let ...)` |                    整数比较、局部变量绑定                    |                GNU make 4.4+ 才有，老环境慎用                |

### 9.记忆要点

1. **参数个数差异**
   有的参数用逗号分隔多个（如 `subst from,to,text`），有的参数是单词列表（如 `$(sort ...)`），有的参数本身就是表达式（如 `$(if ...)` 里可以嵌套）。
2. **参数判断**
   参数之间用逗号隔开，如果有两个变量之间用空格隔开，他们属于一个参数列表也算一个参数
3. **全部是"展开时求值"**
   make 先展开所有函数，得到纯字符串，然后才用于规则/命令。
4. **最常用的就这几个**
   `wildcard`、`patsubst`（或 `:%.c=%.o` 简写）、`foreach`、`if`、`filter`、`notdir`、`dir`、`shell`、`error/warning/info`。写板级/驱动 Makefile 基本离不开 `obj-y += xxx.o` + 这些函数的组合。
5. **查看完整手册**
   `info make` 或 GNU Make Manual 的 **Functions** 章节。

### 10.参数的一个重要澄清

**`$(wildcard ...)` 的参数就是 `.*.cmd $(foreach ...)` 这一整串。** 

make 函数参数只按逗号分隔，不按空格；`foreach`内部的逗号在嵌套括号里，不算`wildcard`的参数分隔符。展开后是一串空格分隔的词，`wildcard`把**每个词**当作一个 pattern 去匹配磁盘：

|        输入词        |    类型    |         `wildcard`的行为         |
| :------------------: | :--------: | :------------------------------: |
|       `.*.cmd`       | 通配符模式 | 展开为所有匹配的文件（数量不定） |
| `common/.main.o.cmd` | 具体文件名 |    存在 → 保留；不存在 → 丢掉    |

展开顺序：由内向外 —— 先展开 `$(foreach)`/`$(dir)`/`$(notdir)`，`wildcard`最后拿到完全展开的字符串。

### 11.流程图小结

```makefile
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

---

## 	4.makefile自定义函数 

```makefile
#定义
define 函数名 
	函数体		#用$(0….9)来获取第n个参数，第0个参数是函数名字
endef
#调用：
	$(call 函数名 参数）
```

## 	5.make命令的使用

```shell
make #默认访问文件名为makefile的文件

make 变量1=值1 变量2=值2 [目标] #向 Makefile 传递参数（变量）

make -f file#make文件名为file的文件

make -i #忽略所有执行出现的错误导致的停止，即使出错了，也会接着往下执行，不会在出错的地方直接停止了。这很重要，特别是当一个工程要编译几个小时时，编译了好久因为出错停了又得重来。

make -n #只打印要执行的命令有哪些，并不执行这些命令

make -s #只执行，不打印命令

make -w #当工程庞大分了好多文件目录时，打印当前的目录来表示当前编译进入到哪个目录了，告诉你从哪跳到哪了

make -C #指定哪个目录下有makefile(默认加了-w的功能)，让make去执行它    
```

##  	6.分目录管理源码

```makefile
#*.c-->src目录
#*.o-->obj目录
#当前目录的make执行makefile时，也去执行其他目录下的makefile则当前目录下的makefile要用到   
make -C 目录名1 目录名2 目录名3.... 这条shell语句，又因为执行shell语句要用到目标：依赖这样的格式,则应为：
1.SRCDIR = ./src/
	$(SRCDIR):
		make -C $@
		#执行结果：
		'src/'已是最新
#一切皆文件，目录也是文件,则会出现重名的问题，解决办法，以SRCDIR为目标的语句中添加依赖，依赖不能是目录，不然也会出现同样问题。
 $(SRCDIR): ECHO
	make -C  $@
 ECHO:
 	echo Hello
 	
 	
2.利用.PHONY解决,一个makefile文件中只能使用一次
 .PHONY:$(SRCDIR)
  $(SRCDIR):
		make -C  $@
		
3.
export 变量1，变量2，变量…..#将当前makefile的变量传递给其他makefile文件使用
如果把带有路径的文件名赋值给变量，当该变量在一个文件夹中定义赋值，而在另外一个文件夹中使用时应该用绝对路径。
```

## 	7.makefile总结

- **默认条件**
  假设目标名是test，如果当前目录中有test.c（一定要是.c文件才可），即存在以目标名为前缀，.c为后缀的C源文件，哪怕有test.c,test.txt并存，只要有test.c,则都算满足默认条件。

- **默认语句**
  $(CC)  $(FLAGS)  xxx.c -o xxx，CC FLAGS可以赋值，也可以使用默认值，其中FLAGS没有默认值。

- **执行步骤**

  假设当前目录下有add.c   a.txt  b.txt  makefile  sub.c  test.c  test.txt这些文件


1.**只有一个目标，无依赖无命令**

```makefile
###1.目标满足默认条件
	test:
		#有以test为前缀,.c为后缀得test.c，且命令缺省，满足默认条件
		#执行结果：
		cc     test.c   -o test


###2.目标不满足默认条件
	a:
		#不存在以a为前缀,.c为后缀得a.c，不满足默认条件，且命令缺省。
		#执行结果：
		make: 对“a”无需做任何事。
```

**2.有目标，有依赖，但没命令**		

```makefile
	###1.目标满足默认条件，且缺省命令，且依赖当前目录存在，执行默认语句
		test:a.txt
			#执行结果：
			cc     test.c a.txt   -o test
			
			
	###2.目标满足默认条件，且缺省命令，但依赖不存在当前目录，则需要make依赖，创造成功但命令缺省，则执行默认语句，不过默认语句要加上依赖文件
		#2.1如果依赖作为目标满足默认条件，则先执行默认语句创造依赖，再执行本次make语句
		test:add
			#执行结果：
			cc     add.c   -o add
			cc     test.c add   -o test
		#2.2如果依赖作为目标不满足默认条件，且没有制作依赖的语句，则全报错
		test:a
			#执行结果：
			make: *** 没有规则可制作目标“a”，由“test” 需求。 停止。
		#2.3如果依赖作为目标不满足默认条件，且有制作依赖的语句，先执行创造依赖的语句，制作依赖的语句可以当作新的语句来继续进行判断是否制作成功，若成功，再执行本次make默认语句
		test:a
		a:add
			#执行结果：
			cc     add.c   -o add
			cc     test.c a   -o test
	
	
	
	###3.目标不满足默认条件，且缺省命令，先创建依赖，依赖创建规则同上，又因为缺省命令，所以此次make语句相当于不会执行，不是报错(此时叫伪目标)。
		a:add
			#执行结果：
			cc     add.c   -o add
		a:b
			#执行结果：
			make: *** 没有规则可制作目标“b”，由“a” 需求。 停止。
		a:b
		b:add
			#执行结果：
			cc     add.c   -o add
```

3**.有目标，有命令，但没依赖。**

不存在满不满足默认条件的问题，因为满足默认条件了，只是为了执行默认语句命令，都有了命令，肯定不是默认命令了啊，故目标满不满足默认条件不影响

```makefile
###无论目标满不满足默认条件，都会按照命令执行
	a:
		@echo hello
		#执行结果：
		hello
		
	test:
		@echo hello
		#执行结果：
		hello
		
		
###如果目标与当前目录下文件重名：解决办法.PHONY
	test.c:
		@ehoc hello
		#执行结果:
		make: “test.c”已是最新。
	
	.PHONY:test.c
	test.c:
		@ehoc hello
		#执行结果:
		hello
```

4.**有目标，有命令，有依赖**

```makefile
#先制作依赖，再根据命令不是默认命令制作目标!!,不存在满不满足默认条件的问题，因为满足默认条件了，只是为了执行默认语句命令，都有了命令，肯定不是默认命令了啊，故目标满不满足默认条件不影响
```

makefile每条语句生成的目标都只是作为中间结果，和最终的一个目标，这些中间结果和最终目标文件会不会在当前目录下创建是要看以它们为目标的语句所用的shell命令是不是会去在当前目录下创建文件。不会在当前目录下创建目标文件的语句：叫做伪目标语句。

# include命令

```http
https://www.cnblogs.com/cuckoos/articles/5049984.html
```
