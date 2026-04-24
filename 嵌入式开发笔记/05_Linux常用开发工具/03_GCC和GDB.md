

# GCC编译器

## 1.GCC概念

`GNU C Complie`编译器单独指C编译器，但现在多指GNU Complie Collection ，是一个工具集合，是开源跨平台编译器套件，是 Linux / 类 Unix 系统的默认编译器，支持 C/C++/Go/Objective-C 等多语言，适配 x86、ARM、RISC-V 等几乎所有主流硬件架构，核心作用是将高级语言源码转换为目标平台的机器码。

## 2.GCC编译过程

 ```bash
 gcc -E	#(预处理)    .c ---> .i
 gcc -S	#(编译:检查语法错误) .i ---> .s(汇编文件)
 gcc -c	#(汇编)     .s-->.o(二进制文件)
 gcc		#(链接)         各种.o-->可执行文件.elf
 
 #所有的.c文件都有经过预处理，编译，汇编三个步骤。
 #这三个步骤独立，不依赖不依靠其他文件的功能，库函数，即不是说别的函数调用了stdio库，我就可以不调了。
 #最终独自生成.o文件，然后所有.o文件链接形成一个可执行文件。
 ```

### 注意事项

```bash
-已知a为可执行程序，b为动态库，c为动态库,且b编译时要链接c
-a编译时既要链接b也要链接c，不依赖不依靠其他文件的功能，库函数
-因为b链接c只是为了编译通过不报错而正确生成b.so的文件，但b.so文件并未包含c的代码，因此编译a时仍然需要链接c
```

## 3.高频核心参数

### 1.基础控制

```bash
gcc -o <输出文件>  #指定输出名
	-c #仅编译不链接，生成.o
	-g #生成调试信息，供 GDB 使用
gcc -g     #进行调试---->gdb ./a.out
```

### 2.优化等级

```bash
-O0 #（无优化，默认/调试用）
-O1/-O2/-O3 #优化递增，发布用
-Os #体积优化

gcc -O/O2  #优化代码，同volatile里优化意思一样
```

### 3.警告检查

```bash
-Wall #开启所有常见警告
-Werror #将警告视为错误，强制修复
```

### 4.头文件 / 库链接

```bash

-I <路径> #指定头文件搜索路
-L <路径> #指定库搜索路径）
-l <库名> #链接指定库，如-lm链接数学库

-I	#（大写i） 指定头文件的路径：指定头文件
gcc -l	#（小写L） 库名 ：链接库
gcc tets.c -lpthread
gcc -L 	#指定库的路径
```

### 5.多文件编译

直接指定多个源码文件（如`gcc a.c b.c -o app`），或先编译为.o 再链接（适合大型项目）。

## 4.配套工具

```shell
nm #查看符号表
objdump #反汇编 / 分析二进制
gcov #代码覆盖率分析
ldd #查看可执行文件依赖的库
```

## 		5.条件编译

好处：通过命令传宏来决定执行哪些代码，省的总注释来注释去的，即相当于一个开关，常用于底层开发代码中。

```c
//1. 根据宏是否被定义来进行条件判断并进行条件编译
#ifdef
#ifndef

#endif
    
    
//2.根据宏的值。。。。。。。
#if VALUE//根据宏的值为真还是假来
#endif

gcc xxx.c -D 宏1    //相当于给程序里添加宏定义的语句即#define 宏1 
gcc xxx.c -D VAULE = 1   //把value的值赋值为1 
```

------

# 交叉编译工具链

## 1. 核心定位

针对 “宿主机 - 目标机异构” 场景的编译工具集：宿主机（如 x86_64 Linux）编译出能在目标机（如 ARM Linux、裸机 MCU、RISC-V 嵌入式）运行的程序，解决 “本地编译的程序无法在异构平台运行” 的核心问题。

## 2. 核心组成

- **交叉 GCC**
  适配目标架构的编译器（如 arm-linux-gnueabihf-gcc）

- **交叉汇编器（as）**
  处理目标架构的汇编

- **交叉链接器（ld**）
  处理目标架构的链接

  ```BASH
  arm-linux-ld -Txx.lds -o a.out main.o math.o
  #xx.lds,链接脚本
  ```

- **Binutils**
  交叉版本的 objcopy、objdump、ar 等二进制工具

- **目标平台 C 库**
  glibc（完整 Linux）、uClibc/musl（轻量嵌入式）、裸机无 C 库（需链接脚本）；

- **目标平台头文件 / 库文件**
  与目标机系统匹配的头文件和库。

## 3. 命名规则

```bash
#格式
<架构>-<厂商>-<系统>-<ABI/特性>-<编译器>

#示例：
arm-linux-gnueabihf-gcc： #ARM 架构、Linux 系统、EABI（嵌入式 ABI）、硬浮点（hf）；
riscv64-unknown-elf-gcc： #RISC-V64 架构、通用厂商、无操作系统（elf）。
```

## 4. 获取与使用要点

### 1.获取方式

1. **现成工具链**
   Linaro（ARM）、Buildroot/Yocto（定制嵌入式）、芯片厂商提供（如 STM32）；
2. **手动构建**
   通过 crosstool-NG（主流）定制，适配特定架构 / 内核 / C 库版本。

### 2.使用关键

1. **工具链前缀替换**
   用交叉 GCC（如`arm-linux-gnueabihf-gcc`）代替原生 gcc；

2. 匹配 ABI / 架构
   硬浮点工具链不能编译软浮点目标机程序（否则运行崩溃）；

3. **指定 sysroot**
   `--sysroot=<路径>`（链接目标机的库和头文件）；

4. **裸机编译**
   需指定链接脚本（`-T <链接脚本.lds>`），无操作系统时需处理启动文件（startup.s）。

   ```BASH
   arm-linux-ld -Txx.lds -o a.out main.o math.o
   #xx.lds,链接脚本
   ```

# arm-linux-gnueabihf-xx

## 1.前缀说明

```bash
arm-linux-gnueabihf-<工具名>
- arm #目标架构为 ARM（32 位，若为 arm64 则前缀为aarch64-linux-gnu-）；
- linux #目标系统为 Linux（裸机则为arm-none-eabi-）；
- gnueabihf #ABI 类型（gnu EABI + 硬浮点（hf），软浮点为gnueabi）；
- 工具名 #gcc/as/ld/objdump 等核心工具。
```

## 2.高频工具命令

### 1. 交叉编译器

```bash
arm-linux-gnueabihf-gcc
#核心作用：将 C/C++ 源码编译为 ARM Linux 可执行程序 / 目标文件，是工具链核心。
```

| 常用命令 / 选项     |                           作用说明                           |                             示例                             |
| ------------------- | :----------------------------------------------------------: | :----------------------------------------------------------: |
| **基础编译**        |                                                              |                                                              |
| `-o <输出文件>`     |          指定输出文件名称（可执行程序 / 目标文件）           |           `arm-linux-gnueabihf-gcc test.c -o test`           |
| `-c`                |      仅编译不链接，生成`.o`目标文件（大型项目分步编译）      |        `arm-linux-gnueabihf-gcc -c test.c -o test.o`         |
| `-g`                |            生成调试信息（供 gdb 调试，调试必加）             |         `arm-linux-gnueabihf-gcc test.c -o test -g`          |
| **优化与警告**      |                                                              |                                                              |
| `-O0/-O2/-Os`       | 优化等级：-O0（无优化，调试用）、-O2（性能优化，发布用）、-Os（体积优化） |         `arm-linux-gnueabihf-gcc test.c -o test -O2`         |
| `-Wall`             |               开启所有常见警告（提前发现问题）               |        `arm-linux-gnueabihf-gcc test.c -o test -Wall`        |
| `-Werror`           |              将警告视为错误（强制修复所有警告）              |    `arm-linux-gnueabihf-gcc test.c -o test -Wall -Werror`    |
| **头文件 / 库链接** |                                                              |                                                              |
| `-I <路径>`         |                指定头文件搜索路径（优先搜索）                |    `arm-linux-gnueabihf-gcc test.c -o test -I ./include`     |
| `-L <路径>`         |                      指定库文件搜索路径                      |      `arm-linux-gnueabihf-gcc test.c -o test -L ./lib`       |
| `-l<库名>`          |           链接指定库（省略 lib 前缀和.so/.a 后缀）           |  `arm-linux-gnueabihf-gcc test.c -o test -lm`（链接数学库）  |
| `--sysroot=<路径>`  | 指定目标机根文件系统（链接目标机的库 / 头文件，嵌入式核心）  | `arm-linux-gnueabihf-gcc test.c -o test --sysroot=/opt/arm-rootfs` |
| **架构 / ABI 适配** |                                                              |                                                              |
| `-march=armv7-a`    |         指定目标 ARM 架构版本（如 armv7-a/armv8-a）          |   `arm-linux-gnueabihf-gcc test.c -o test -march=armv7-a`    |
| `-mfpu=neon`        |       指定浮点运算单元（硬浮点需匹配，如 neon/vfpv4）        |     `arm-linux-gnueabihf-gcc test.c -o test -mfpu=neon`      |
| `-mfloat-abi=hard`  | 显式指定硬浮点 ABI（与工具链 hf 后缀匹配，软浮点为 soft/softfp） |  `arm-linux-gnueabihf-gcc test.c -o test -mfloat-abi=hard`   |
| **裸机编译专属**    |                                                              |                                                              |
| `-T <链接脚本.lds>` |             指定链接脚本（定义内存布局、段分配）             | `arm-linux-gnueabihf-gcc startup.s test.c -o test.elf -T link.lds` |
| `-nostdlib`         |             不链接标准库（裸机无操作系统时使用）             | `arm-linux-gnueabihf-gcc test.c -o test.elf -T link.lds -nostdlib` |

### 2. 交叉汇编器

```bash
arm-linux-gnueabihf-as
#核心作用：将 ARM 汇编代码（.s）转为二进制目标文件（.o）
```

|    常用选项     |             作用说明             |                             示例                             |
| :-------------: | :------------------------------: | :----------------------------------------------------------: |
| `-o <输出文件>` |         指定输出.o 文件          |       `arm-linux-gnueabihf-as startup.s -o startup.o`        |
|      `-g`       |        生成汇编级调试信息        |      `arm-linux-gnueabihf-as startup.s -o startup.o -g`      |
| `-march=armv7`  | 指定 ARM 架构版本（与 gcc 匹配） | `arm-linux-gnueabihf-as startup.s -o startup.o -march=armv7` |

### 3. 交叉链接器

```bash
arm-linux-gnueabihf-ld
#核心作用：将多个.o 目标文件 + 库文件链接为可执行程序 / ELF 文件（裸机 / 应用层均可用）。
```

|       常用选项        |              作用说明               |                             示例                             |
| :-------------------: | :---------------------------------: | :----------------------------------------------------------: |
|    `-o <输出文件>`    | 指定输出文件（如 elf / 可执行程序） |    `arm-linux-gnueabihf-ld test.o startup.o -o test.elf`     |
|  `-T <链接脚本.lds>`  |      指定链接脚本（裸机核心）       | `arm-linux-gnueabihf-ld test.o startup.o -o test.elf -T link.lds` |
|      `-L <路径>`      |           指定库搜索路径            |     `arm-linux-gnueabihf-ld test.o -o test -L ./lib -lm`     |
|      `-l<库名>`       |             链接指定库              |                             同上                             |
|  `--sysroot=<路径>`   |      链接目标机根文件系统的库       | `arm-linux-gnueabihf-ld test.o -o test --sysroot=/opt/arm-rootfs` |
| `-Map <映射文件.map>` |  生成链接映射文件（分析内存分布）   | `arm-linux-gnueabihf-ld test.o -o test -T link.lds -Map test.map` |

### 4. 二进制分析工具

```bash
arm-linux-gnueabihf-objdump
#核心作用：反汇编、分析 ELF 文件 / 目标文件，定位内存布局、指令问题（调试 / 优化核心工具）。
```

|  常用选项  |               作用说明                |                         示例                         |
| :--------: | :-----------------------------------: | :--------------------------------------------------: |
|    `-d`    |       反汇编可执行段（代码段）        |   `arm-linux-gnueabihf-objdump -d test > test.asm`   |
|    `-S`    | 源码 + 汇编混合反汇编（需编译加 - g） | `arm-linux-gnueabihf-objdump -S test > test_src.asm` |
|    `-h`    |  查看 ELF 文件段信息（大小 / 地址）   |        `arm-linux-gnueabihf-objdump -h test`         |
|    `-x`    | 查看 ELF 文件所有头信息（符号 / 段）  |        `arm-linux-gnueabihf-objdump -x test`         |
| `-j .text` |    仅分析指定段（如.text 代码段）     |    `arm-linux-gnueabihf-objdump -d -j .text test`    |

### 5. 符号表工具

```bash
arm-linux-gnueabihf-nm
#核心作用：查看 ELF 文件 / 目标文件的符号表（函数名、变量名、地址、类型）。
```

| 常用选项 |              作用说明               |               示例               |
| :------: | :---------------------------------: | :------------------------------: |
|   `-a`   |      显示所有符号（包括行号）       | `arm-linux-gnueabihf-nm -a test` |
|   `-l`   |  显示符号对应的源码行号（需 - g）   | `arm-linux-gnueabihf-nm -l test` |
|   `-u`   | 仅显示未定义符号（缺失的库 / 函数） | `arm-linux-gnueabihf-nm -u test` |
|   `-s`   |            显示符号大小             | `arm-linux-gnueabihf-nm -s test` |

### 6. 二进制转换工具

```bash
arm-linux-gnueabihf-objcopy
#核心作用：转换 ELF 文件格式（如 ELF→bin，裸机烧录核心）、提取 / 修改段数据。
```

|     常用选项      |                作用说明                |                             示例                             |
| :---------------: | :------------------------------------: | :----------------------------------------------------------: |
|    `-O binary`    | 将 ELF 文件转为二进制 bin 文件（烧录） |  `arm-linux-gnueabihf-objcopy test.elf -O binary test.bin`   |
|    `-j .text`     |     仅提取指定段（如.text 到 bin）     | `arm-linux-gnueabihf-objcopy test.elf -j .text -O binary text.bin` |
|       `-S`        |       移除符号表（减小文件体积）       |   `arm-linux-gnueabihf-objcopy -S test.elf test_strip.elf`   |
| `--gap-fill=0xff` |        填充空白区域（烧录对齐）        | `arm-linux-gnueabihf-objcopy test.elf -O binary test.bin --gap-fill=0xff` |

### 7. 库管理工具

```bash
arm-linux-gnueabihf-ar
#核心作用：创建 / 修改 / 提取静态库（.a 文件），嵌入式常用静态链接减小依赖。
```

|    常用命令 / 选项    |             作用说明             |                         示例                          |
| :-------------------: | :------------------------------: | :---------------------------------------------------: |
| `rc <库名.a> <o文件>` | 创建静态库（r = 替换，c = 创建） | `arm-linux-gnueabihf-ar rc libtest.a test1.o test2.o` |
|     `t <库名.a>`      |     查看静态库包含的.o 文件      |         `arm-linux-gnueabihf-ar t libtest.a`          |
|     `x <库名.a>`      |      提取静态库中的.o 文件       |         `arm-linux-gnueabihf-ar x libtest.a`          |
| `rs <库名.a> <o文件>` | 替换静态库中的.o 文件并更新索引  |     `arm-linux-gnueabihf-ar rs libtest.a test1.o`     |

### 8. 交叉调试器

```bash
arm-linux-gnueabihf-gdb
#核心作用：调试交叉编译的 ARM 程序（本地 / 远程），需配合目标机gdbserver。
```

|       常用命令 / 选项       |             作用说明             |             示例（远程调试）             |
| :-------------------------: | :------------------------------: | :--------------------------------------: |
|          启动 GDB           |          加载待调试程序          |      `arm-linux-gnueabihf-gdb test`      |
| `target remote <IP>:<端口>` |       连接目标机 gdbserver       | `(gdb) target remote 192.168.1.100:1234` |
|       `file <程序名>`       | 加载符号表（确保与目标程序一致） |            `(gdb) file test`             |
|    `set sysroot <路径>`     |  指定目标机根文件系统（解析库）  |   `(gdb) set sysroot /opt/arm-rootfs`    |
|          `b main`           |     设置断点（函数 / 行号）      |   `(gdb) b main` / `(gdb) b test.c:10`   |
|             `r`             |  运行程序（本地）/ 触发远程运行  |                `(gdb) r`                 |
|            `s/n`            |       单步进入 / 单步跳过        |          `(gdb) s` / `(gdb) n`           |
|        `p <变量名>`         |            打印变量值            |       `(gdb) p a` / `(gdb) p *ptr`       |
|            `bt`             |      查看调用栈（定位崩溃）      |                `(gdb) bt`                |
|           `quit`            |             退出 GDB             |               `(gdb) quit`               |

### 9. 目标机调试服务

```bash
gdbserver#ARM 端
#核心作用：在 ARM 目标机上运行，监听端口并配合宿主机交叉 GDB 调试（无需目标机装完整 GDB）。
```

|            常用命令            |          作用说明          |                         示例                          |
| :----------------------------: | :------------------------: | :---------------------------------------------------: |
| `gdbserver <IP>:<端口> <程序>` | 启动调试服务，监听指定端口 |    `gdbserver :1234 ./test`（监听本机 1234 端口）     |
|   `gdbserver --attach <PID>`   |    附加到已运行进程调试    | `gdbserver :1234 --attach 123`（调试 PID=123 的进程） |
|  `gdbserver core <core文件>`   |    调试 core dump 文件     |           `gdbserver :1234 core test.core`            |

## 3.典型场景组合示例

### 1.ARM Linux 应用编译 + 调试

1. 交叉编译（带调试信息）：

   ```bash
   arm-linux-gnueabihf-gcc app.c -o app -g -Wall --sysroot=/opt/arm-rootfs -O0
   ```

2. 目标机启动 gdbserver：

   ```bash
   # ARM板端执行
   gdbserver :1234 ./app
   ```

3. 宿主机交叉 GDB 调试：

   ```bash
   # 宿主机执行
   arm-linux-gnueabihf-gdb ./app
   (gdb) target remote 192.168.1.100:1234  # 连接ARM板
   (gdb) b main                            # 设断点
   (gdb) r                                 # 运行程序
   (gdb) p argc                            # 查看参数
   ```

### 2.ARM 裸机程序编译（STM32 为例）

```bash
# 汇编启动文件→目标文件
arm-linux-gnueabihf-as startup_stm32.s -o startup_stm32.o -march=armv7-m
# C源码→目标文件
arm-linux-gnueabihf-gcc main.c -o main.o -c -g -march=armv7-m -mthumb -O0
# 链接（指定链接脚本）→ELF文件
arm-linux-gnueabihf-ld startup_stm32.o main.o -o stm32_app.elf -T stm32.lds -Map stm32.map
# ELF→BIN（烧录文件）
arm-linux-gnueabihf-objcopy stm32_app.elf -O binary stm32_app.bin
# 反汇编分析
arm-linux-gnueabihf-objdump -S stm32_app.elf > stm32_app.asm
```

## 4.关键注意事项

1. **工具链匹配**
   硬浮点（hf）工具链不能编译软浮点目标机程序，否则运行崩溃；
2. **调试禁用优化**
   `-O0`是调试前提，`-O2/-O3`会优化代码导致断点 / 变量查看错位；
3. **sysroot 必选**
   嵌入式编译时`--sysroot`需指向目标机根文件系统，否则链接宿主机库导致程序无法运行；
4. **裸机链接脚本**
   必须匹配芯片内存布局（如 FLASH/RAM 地址），否则程序无法启动；
5. **gdb 版本一致**
   交叉 GDB 与目标机 gdbserver 版本需匹配，否则连接失败。

# 		GDB调试

## 1.进入调试模式

```shell
gdb 可执行文件名
```

## 2.设置断点

```shell
break 
#设置断点，后面可以跟函数名或行号，表示在某一行打断点
```

## 3.运行程序

```shell
1.
start  #进入程序的入口处，等待执行程序
continue #statrt后往后顺序执行，直到遇到一个断点处

2.
r(run):#直接让代码运行至断点处

3.
n(next) #从断点处开始一条一条的执行
```

## 4.打印信息

```shell
print (&)变量名 #查看变量的值，加上&查看变量的地址

x /nxb &s #打印变量s在内存存储的具体的内容，格式为从结构体首地址开始分别输出n个字节的内容，格式显示为16进制。

bt		#打印调用栈

bt		#查看栈

l(list)	 #不记得代码了，敲一个l显示一段代码内容。

Edit



Step

Help

Quit
```

## 5.GDB调试程序

```shell
gcc -g xxx.c #如果程序出错（非语法错误，比如越界，段错误)，则会生成core和a.out文件

gdb a.out core #（core文件):在core文件里查看出错原因

gdb a.out -p 进程pid	#调试正在运行的进程
```

![image-20240621124813998](..\figure\image-20240621124813998.png)

![image-20240621125023445](..\figure\image-20240621125023445.png)

# GDB源码包下载

对于嵌入式软件开发调试工具没有现成的，且嵌入式系统比较繁杂，gdbserver需要根据目标系统单独编译。gdb的源码包下载地址为：Index of /gnu/gdb

一般Linux发行版中都有一个可以运行的GDB，但不能直接使用该发行版中的GDB来做远程调试，而需要获取GDB的源代码包，针对arm平台作一个简单配置，重新编译得到相应GDB。

## 1.解压源码包

```bash
tar -zxvf gdb-7.12.tar.gz  
```

## 2.生成Makefile

```bash
cd gdb-7.12/  #进入源码目录
mkdir /usr/local/arm-gdb#创建一个安装目录
./configure --target=arm-linux --prefix=/usr/local/arm-gdb#执行配置文件
#注意：
	--target： #指定编译环境，一般设置为交叉编译器前缀。
	--prefix： #指定安装路径，可自己任意指定合法路径
```

## 3.编译与安装

```bash
make#编译

make install#安装
#安装结束后会在指定的安装目录下生成三个文件夹，里面包含可执行文件、头文件、动态库文件等
```

## 4.填加到环境变量

将生成的bin文件添加到环境变量中：

```bash
vim /etc/profile
export PATH=$PATH:/usr/local/arm-gdb/bin         #添加在文件末尾
source /etc/profile           #生效环境变量
echo $PATH                     #检查环境变量
#注意：
1.也可以放在其它环境变量下，例如：/usr/local/bin、/usr/bin等
2.生成的文件可删除不需要用到的，比如，此处只需要使用arm-linu-gdb可执行文件，则你可只需将此文件添加到环境变量即可，其他文件可直接删除。
```

# GDB远程调试

## 1.远程调试原理

在 GDB 远程调试的整个流程中，**只有 ARM 开发板上的`app`程序在真正运行**，宿主机（服务器）上的`app`仅作为 “符号表载体”，全程不会被执行，以下是精准拆解：

### 1.核心运行逻辑

“运行体” 与 “解析体” 分离。

|  位置  |        文件角色        | 是否运行 | 核心作用                                                     |
| :----: | :--------------------: | :------: | ------------------------------------------------------------ |
| 开发板 | `/home/root/test/app`  |   ✅ 是   | 被 gdbserver 加载到板端内存，真正执行指令、占用 CPU / 内存，响应调试指令（断点、单步）； |
| 宿主机 | `/opt/arm_project/app` |   ❌ 否   | 仅被交叉 GDB 读取**调试符号表**（行号、函数名、变量地址映射），不加载、不执行； |

### 2.可视化运行流程

```plaintext
宿主机                          网络                          开发板
┌─────────────────────┐        ┌───────┐        ┌─────────────────────┐
│ 交叉GDB + 本地app（仅读符号） │◄─────►│ TCP/IP │ gdbserver + 板端app（实际运行） │
└─────────────────────┘        └───────┘        └─────────────────────┘
     ▲                                          ▲
     │ 解析“源码指令”为“内存地址指令”           │ 执行指令 + 回传内存/寄存器状态
     │ （如：b main → 转换为0x12345678断点）    │ （如：暂停在0x12345678，回传a=10）
     └──────────────────────────────────────────┘
```

### 3.关键佐证

1. **进程仅出现在开发板**

   调试时，在开发板执行`ps -ef | grep app`，能看到`app`进程（由 gdbserver 拉起）；
   而宿主机执行`ps -ef | grep app`，只会看到`arm-linux-gnueabihf-gdb`进程，不会有`app`的运行进程。

2. **资源占用仅在开发板**

   `app`运行时消耗的 CPU、内存、外设（如 ARM 板的 GPIO、串口）等资源，均是开发板的资源，宿主机仅消耗 GDB 本身的少量资源（解析符号、下发指令）。

3. **程序崩溃仅影响开发板**

   若调试中`app`触发段错误，崩溃的是开发板上的`app`进程，宿主机 GDB 仅收到 “程序崩溃” 的通知，自身不会受影响。

### 4.常见误区纠正

❌ 误区：“宿主机 GDB 启动`./app`后，程序会在主机运行，再同步到板端”

✅ 正解：宿主机 GDB 的`r`（run）指令，本质是**下发 “启动板端 app” 的指令给 gdbserver**，而非在主机运行 app—— 因为宿主机是 x86 架构，ARM 架构的`app`根本无法在主机运行（指令集不兼容）。

## 2.安装gdbserver

###  1.生成Makefile

```bash
cd gdb-7.12/gdb/gdbserver/  #进入源码目录

./configure --target=arm-linux-gnueabi --host=arm-linux#执行配置文件    

#注意：
--target#指定目标平台，如目标平台为ARM。
--host#指定宿主机运行的是arm-linux-gdb
```

### 2.编译与安装

```bash
make
#编译gdbserver不需要执行make install命令，因为make之后在当前目录下会生成可执行程序gdbserver。
```

### 3.拷贝到目标平台

```bash
1.拷贝之前先更改gdbserver读写权限：chmod 777 gdbserver 
2.将可执行文件gdbserver拷贝到目标平台的/usr/local/bin/目录下。至此，远程调试环境已经搭建完成。
```

## 3.调试流程

### 1.检查网络是否正常

登入虚拟机和开发板的Linux系统，执行如下操作：

1. 开发板ping主机

2. 开发板ping虚拟机

3. 主机ping开发板

4. 虚拟机ping开发板

5. 保证相互之间均可以ping通。

6. 注意：主机ip、开发板ip和虚拟机ip地址设置在同一个网段内。

### 2.编辑和编译测试代码

#### 1.测试代码

```c++
#include <iostream>
#include <string>
using namespace std;
void fun(int &a, int &b)
{
     a = b = 10;
}
void my_fun(int &a, int &b)
{
    a > b ? (a += b) : (b -= a);
    fun(a, b);
 }
int main()
{
    int a = 13， b = 16;
    my_fun(a,b);
    cout<< "a = " << a << endl;
    cout<< "b = " << b << endl;
    return(0);
}                        
```

#### 2.编译代码

```bash
-g #设置带调试信息的程序
```

#### 3.设置读写权限

```bash
#修改可执行二进制文件test读写权限
chmod 777 test
```

#### 4.下载文件到开发板

下载可执行二进制文件到开发板的工作目录下。文件下载方法：在Linux系统编译好的可执行文件先下载到window，然后再从window下载到开发板。（下载操作方式众多，选择自己习惯的就行）

### 3.启动调试环境

```bash
宿主机IP：192.168.xxx.xxx
开发板IP：192.168.xxx.xxx
```

1. 开发板上运行gdbserver

   ```bash
   gdbserver 192.168.xxx.xxx:2001 test
   ```

2. 宿主机上运行arm-linux-gdb

   ```bash
   arm-linux-gdb test
   ```

3. 连接gdbserver

   ```bash
   (gdb) target remote 192.168.xxx.xxx:2001
   ```

### 4..gdbinit脚本

每次启动gdb时都要在gdb命令行上手动输入指令，去连接目标机，操作上显得及其麻烦。而使用.gdbinit脚本则可以轻松解决此事。gdb在启动的时候，会在当前工作目录下查找 ".gdbinit" 这个文件，并把它的内容作为gdb命令进行解释，所以如果把脚本命名为".gdbinit"，这样在启动的时候就会处理一些常用的命令。

1. 在”~"目录下添加一个.gdbinit， 默认gdb初始化时会调用这个文件。

2. 编辑该.gdbinit, 在文件中加上 set auto-load safe-path xxx

   ```bash
   set auto-load safe-path  /home/tanglg/workspace_tlg/
   #指定了/home/tanglg/workspace_tlg/路径为gdb的安全加载路径，即后续可在该路径下的任何子路径下均可加载.gdbinit文件并解释文件内容。
   
   #如果不在“~”目录下指定一个安全路径的话，会在使用.gdbinit时，报如下错误：
   File "/home/tanglg/workspace_tlg/myapp/test/.gdbinit" auto-loading has been declined by your `auto-load safe-path' set to "$debugdir:$datadir/auto-load".
   ```

3. 在/home/tanglg/workspace_tlg/myapp/test/（即要Debug的目录）下，同样添加一个.gdbinit

4. 编辑该.gdbinit,添加你需要的一些指令，例如

   ```bash
   target remote 192.168.xxx.xxx:2001
   #当然在.gdbinit文件中你可以添加别的指令
   #.gdbinit编写好并保存后，就可以直接输入arm-linux-gdb test 开始gdb调试了，中途就不用再次主动去连接目标机的gdbserver了。如果需要更改目标机的IP和端口号，只需修改.gdbinit文件。
   ```

5. 至此开发板和虚拟机（宿主机）远程连接起来了。


### 5.开始调试

建立链接后，就可以进行调试了。调试在宿主机端，跟gdb调试方法相同。注意的是要用“c”来执行命令，不能用“r”。因为程序已经在Target Board上面由gdbserver启动了，结果输出是在Target Board端，用SSH(或超级终端)查看。

## 4.调试常用命令

```bash
c    #-->    continue的缩写，作用是程序继续往下执行。
l    #-->    list的缩写，作用是查看程序代码，按回车可显示剩余未显示的代码。
b    #-->    break的缩写，作用是设置断点。如：b xxx.cpp:4表示在xxx.cpp的4行设置断点。
b i  #-->    break info的缩写，作用是查看所有断点信息。
q    #-->    quit的缩写，作用是退出调试  
```

## 5.参考链接

[gdb+gdbserver远程调试环境搭建及调试_gdb server-CSDN博客](https://blog.csdn.net/m0_56121792/article/details/133160361)

# VSCode调试配置

## 1.本地交叉编译代码

### 1.编写测试代码

新建项目目录（如`~/embedded_debug`），创建`main.c`：

```c
#include <stdio.h>

int add(int a, int b) {
    return a + b; // 此处可打断点
}

int main() {
    int x = 10, y = 20;
    int res = add(x, y);
    printf("Result: %d\n", res);
    return 0;
}
```

### 2. 编写 Makefile

在项目目录创建`Makefile`，指定交叉编译器和**`-g`调试参数**（必须加，否则无法调试）：

```makefile
# 交叉编译器（根据实际工具链修改）
CC = arm-linux-gnueabihf-gcc
# 编译参数：-g 生成调试信息，-O0 关闭优化（避免调试时代码乱序）
CFLAGS = -g -O0
# 目标可执行文件名称
TARGET = test_app

# 编译规则
all: $(TARGET)

$(TARGET): main.c
    $(CC) $(CFLAGS) -o $(TARGET) main.c

# 清理编译产物
clean:
    rm -rf $(TARGET)
```

### 3.本地编译

在 VSCode 终端执行编译

```bash
make
```

编译完成后，目录下会生成`test_app`（带调试信息的交叉编译产物）。

## 2.VSCode 自动上传到开发板

通过 VSCode 的`tasks.json`配置「编译后自动上传」，避免手动执行`scp`。

### 1. 生成 tasks.json

- 打开 VSCode，按`Ctrl+Shift+B` → 选择「创建 tasks.json 文件」 → 选择「Others」，生成基础模板。

- 替换为以下内容（修改开发板 IP、用户名、上传路径）

  ```json
  {
      "version": "2.0.0",
      "tasks": [
          // 任务1：编译代码（执行make）
          {
              "label": "build",
              "type": "shell",
              "command": "make",
              "args": [],
              "group": {
                  "kind": "build",
                  "isDefault": true
              },
              "problemMatcher": "$gcc",
              "detail": "交叉编译代码"
          },
          // 任务2：上传编译产物到开发板（依赖build任务）
          {
              "label": "upload",
              "type": "shell",
              "command": "scp",
              "args": [
                  "${workspaceFolder}/test_app", // 本地编译产物路径
                  "root@192.168.1.100:/root/"    // 开发板IP + 目标路径（修改为实际值）
              ],
              "dependsOn": "build", // 先编译，再上传
              "problemMatcher": [],
              "detail": "上传可执行文件到开发板"
          }
      ]
  }
  ```

### 2.执行上传

按`Ctrl+Shift+B` → 选择「upload」，VSCode 会先编译代码，再自动将`test_app`上传到开发板`/root/`目录。

### 3.选择任务运行

在 VSCode 中选择并执行指定任务，核心有**图形化操作**（直观）、**快捷键 / 命令面板**（高效）、**调试前置自动执行**（自动化）三种方式，以下是详细步骤（基于前文的 tasks.json 配置）

#### 1.图形化操作

##### 1. 打开任务列表

- 方式 1：顶部菜单栏 → 「终端」 → 「运行任务」（Terminal → Run Task...）；
- 方式 2：右键点击 VSCode 左侧「资源管理器」的项目文件夹 → 选择「运行任务」。

##### 2. 选择要执行的任务

- 弹出的任务列表中会显示你在`tasks.json`中定义的所有`label`（如`build`、`upload`、`start_gdbserver`、`deploy`等）；
- 点击目标任务（如`upload`），VSCode 会立即执行该任务：
  - 若任务有依赖（如`upload`依赖`build`），会自动先执行依赖任务，再执行目标任务；
  - 执行过程会在底部「终端」面板输出日志，可查看执行结果 / 错误。

##### 3. 可选：跳过任务确认

首次执行任务时，可能会弹出「是否扫描任务输出中的问题」的确认框，选择：

- 「每次运行任务时询问」：每次执行都弹窗；
- 「允许每次运行时扫描」：默认扫描，不弹窗；
- 「不扫描」：仅执行命令，不解析输出（适合纯上传 / 启动类任务）。

#### 2.快捷键 / 命令面板

##### 1. 基础快捷键：Ctrl+Shift+B

- 按下快捷键后，直接弹出任务选择列表（和「运行任务」效果一致）；
- 用方向键↑↓选择目标任务（如`start_gdbserver`），按 Enter 执行。

##### 2. 命令面板精准筛选

- 按下`Ctrl+Shift+P`，打开命令面板；
- 输入关键词「Tasks: Run Task」，回车后弹出任务列表；
- 若任务较多，可在列表中输入任务名称（如`upload`）快速筛选，回车执行。

#### 3.设置默认任务

如果某任务（如`upload`或`deploy`）是日常最常用的，可将其设为「默认构建任务」，按下`Ctrl+Shift+B`后直接执行，无需选择。

##### 1.配置方法

1. 打开`tasks.json`，找到目标任务（如`deploy`），在其`group`字段中添加配置：

   ```json
   {
       "label": "deploy",
       "type": "shell",
       "command": "...",
       "group": {
           "kind": "build", // 归类为「构建任务」
           "isDefault": true // 设置为默认任务
       },
       "detail": "默认部署任务"
   }
   保存后，按下`Ctrl+Shift+B`，VSCode 会直接执行该默认任务（无需选择列表）；
   
   若要取消默认，将`isDefault`改为`false`即可。
   ```

## 3.启动开发板的 gdbserver

gdbserver 的作用是在开发板上监听端口，等待本地 GDB 连接，有两种启动方式：

### 1. 手动启动

1. 本地通过 SSH 登录开发板：

   ```bash
   ssh root@192.168.1.100
   ```

2. 开发板端执行（指定监听端口，如 1234，以及可执行文件路径）：

   ```bash
   cd /root/
   # gdbserver :端口 可执行文件
   gdbserver :1234 ./test_app
   ```

   正常输出：

   ```plaintext
   Process ./test_app created; pid = 12345
   Listening on port 1234
   ```

### 2.VSCode 自动启动

若不想手动登录开发板启动 gdbserver，可在`tasks.json`新增「启动 gdbserver」任务（依赖 upload）：

```json
// 在tasks.json中新增
{
    "label": "start_gdbserver",
    "type": "shell",
    "command": "ssh",
    "args": [
        "root@192.168.1.100",
        "cd /root/ && gdbserver :1234 ./test_app"
    ],
    "dependsOn": "upload",
    "problemMatcher": [],
    "detail": "远程启动gdbserver"
}
```

## 4.配置 VSCode launch.json

`launch.json`是 VSCode 调试的核心配置，用于指定本地交叉 GDB 路径、开发板 gdbserver 地址、调试文件等。

### 1. 生成 launch.json

- VSCode 左侧点击「运行和调试」→ 「创建 launch.json 文件」→ 选择「C/C++ (GDB/LLDB)」。

- 替换为以下内容（根据实际环境修改标注的参数）

  ```json
  {
      // 使用 IntelliSense 了解相关属性。 
      // 悬停以查看现有属性的描述。
      // 欲了解更多信息，请访问: https://go.microsoft.com/fwlink/?linkid=830387
      "version": "0.2.0",//配置文件版本号，VSCode 调试系统的兼容版本，固定写 0.2.0 即可（无需修改），确保配置格式被 VSCode 正确解析。
      "configurations": [//调试配置数组，数组中每个元素是一个独立的调试配置项（可在 VSCode 调试面板下拉框切换选择）。
          
          //这是 VSCode 自动生成的本地 x86 程序调试模板，未实际适配你的 ARM 场景，仅作为默认占位，以下逐字段解析：
          {
              "name": "(gdb) 启动",//"(gdb) 启动"：标识这是本地 GDB 启动调试的默认配置。
              "type": "cppdbg",//"调试器类型cppdbg"：指定用 GDB 调试 C/C++ 程序。
              "request": "launch",//"launch"：启动新程序调试。- launch：启动新程序并调试（最常用）；
                                                           //- attach：附加到已运行的进程进行调试。
              "program": "输入程序名称，例如 ${workspaceFolder}/a.out",//要调试的可执行文件路径（必须是带 -g 调试信息的编译产物）。
              "args": [],//程序运行时的命令行参数，数组形式，每个元素是一个参数。
              "stopAtEntry": false,//是否在程序入口点（main 函数）自动暂停，方便调试启动流程。
              "cwd": "${fileDirname}",//	调试时的工作目录（程序运行时的当前路径），影响文件读写（如相对路径加载配置）。
              "environment": [],//调试时的环境变量，数组形式，每个元素是 {"name": "变量名", "value": "值"}。
              "externalConsole": false,//	是否弹出外部终端窗口运行程序（而非 VSCode 内置终端）。
              "MIMode": "gdb",//指定调试器的机器接口（MI）模式，VSCode 通过 MI 协议和调试器交互；gdb 对应 GDB，lldb 对应 LLDB。
              "setupCommands": [//调试启动前自动执行的 GDB 命令，用于初始化 GDB 行为。
                  {
                      "description": "为 gdb 启用整齐打印",
                      "text": "-enable-pretty-printing",
                      "ignoreFailures": true
                      //GDB 命令（-enable-pretty-printing 启用结构体 / STL 容器的美观打印，避免乱码）
                  },
                  {
                      "description": "将反汇编风格设置为 Intel",
                      "text": "-gdb-set disassembly-flavor intel",
                      //将反汇编代码风格设为 Intel（默认是 AT&T，更符合嵌入式开发者习惯）；
                      "ignoreFailures": true//忽略命令失败。
                  }
              ]
          },
          //===============================================================================================
          {
              "name": "arm-linux-gnueabihf-gcc build",
              "type": "cppdbg",
              "request": "launch",
              //本地带调试符号的可执行文件路径（核心！）。
              //远程调试的关键逻辑：
  			//- 程序实际在开发板运行，但 GDB 的「符号表」依赖本地文件（必须和开发板上的程序完全一致，且编译时加 -g -O0）；
  			//- 若本地无此文件，GDB 无法解析断点、变量名等调试信息。
              "program": "${workspaceFolder}/debian/terminaloop/home/app/dlmsd",
              "args": [],//开发板上程序运行的命令行参数。
              "stopAtEntry": false, //是否在程序入口（main）自动暂停。
              
              //调试时的工作目录（对应开发板上程序的工作目录），确保程序加载相对路径文件（如配置、日志）时路径一致。
              //"cwd": "${workspaceFolder}/build/Debug/bin/ExampleSrv/",
              "cwd": "${workspaceFolder}/debian/terminaloop/home/app/",//通过目录映射到开发板
              "environment": [],
              "targetArchitecture": "arm",//指定目标程序的 CPU 架构，帮助 VSCode 适配调试体验（如反汇编、寄存器显示、内存布局）。
              "externalConsole": false,
              "MIMode": "gdb",
              "logging": {//	调试日志配置，用于排查调试异常（如连接失败、断点不命中）。
                  "engineLogging": false,
              },
              "setupCommands": [
                  {
                      "description": "为 gdb 启用整齐打印",
                      "text": "-enable-pretty-printing",
                      "ignoreFailures": false
                  },
                  {
  //设置 GDB 的「系统根目录」，作用是让本地交叉 GDB 找到 ARM 架构的库文件（如 libc.so），解析库中的符号（避免调试时提示「找不到库符号」）；
  //"text": "set sysroot /home/zhangg/English_698/sg698/debian/terminaloop/home/app"：指向本地存放 ARM 库的目录；
                      "description": "sysroot",
                      //"text": "set sysroot remote",
  1.避免手动同步开发板的库文件到本地（减少维护成本）；
  2.确保 GDB 读取的库文件和开发板上完全一致（避免本地库版本和开发板不一致导致的调试异常）；
  3.简化配置：无需记住本地 sysroot 路径，直接从开发板拉取。
                      //"text": "set sysroot remote:/usr/local/extapps/desktopGui/bin",
                      "text": "set sysroot /home/zhangg/English_698/sg698/debian/terminaloop/home/app",
                      "ignoreFailures": false//此命令必须成功，否则库函数断点无法命中。
                  }
                  //{
                  //    "description": "follow-fork-mode",
                  //    "text": "set follow-fork-mode child",
                  //    "ignoreFailures": true
                  //}
              ],
              
              //C++ STL 容器可视化配置文件路径（.natvis 格式），让调试时 vector/map 等容器以直观形式显示（而非内存地址）。
              "visualizerFile": "/home/zhangg/.vscode-server/stl.natvis",
              
              //是否显示 STL 容器的「友好描述字符串」（如 vector<int> [size=3] 而非原始内存）。
              "showDisplayString": true,
              
              //调试启动前自动执行的 VSCode 任务（被注释），通常用于「自动启动开发板的 gdbserver」（无需手动 SSH 登录启动）。
              //"preLaunchTask": "start gdbserver",
              
              //本地交叉 GDB 的绝对路径（核心！），必须使用和开发板架构匹配的交叉 GDB，而非本地 x86 GDB。
              //"miDebuggerPath": "/mnt/e/component/gdb-8.3.1/install/bin/arm-linux-gnueabihf-gdb",
              "miDebuggerPath": "/opt/ext-toolchain/bin/arm-linux-gnueabihf-gdb",
              //"miDebuggerPath": "/mnt/e/component/gcc-linaro-4.9-2016.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gdb",
              
              
              //开发板上 gdbserver 的IP + 端口，是本地 GDB 连接开发板的核心地址。
              "miDebuggerServerAddress": "192.168.65.187:1234",
              
              //核心转储文件（coredump）路径，用于调试程序崩溃问题（被注释）
              //  "coreDumpPath": "/home/tancan/wango/ModularizationTerminal/build/allwinner/Debug/bin/test.dump"
          }
      ]
  }
  ```

### 2.开始远程调试

#### 1. 启动调试

- 若用「方式 1 手动启动 gdbserver」：确保开发板的 gdbserver 已监听 1234 端口。
- VSCode 左侧「运行和调试」→ 选择「Remote GDB Debug」→ 点击绿色三角启动调试。

#### 2. 调试操作

- 在`main.c`的`add`函数行打断点（点击行号左侧）。
- 调试启动后，程序会停在断点处，可执行以下操作：
  - 单步执行（F10）、步入函数（F11）、步出函数（Shift+F11）。
  - 查看变量：左侧「变量」面板可查看`x`、`y`、`res`的值。
  - 修改变量：右键变量→「设置值」，手动修改后继续执行。
  - 控制台输出：「调试控制台」可查看程序运行日志。

## 5.常见问题排查

1. **gdbserver 连接失败**：
   - 检查开发板 IP 是否可达（`ping 开发板IP`）。
   - 检查开发板防火墙是否关闭（`iptables -F`），或开放 1234 端口。
   - 确保 gdbserver 和本地 GDB 版本匹配（版本差异可能导致连接失败）。
2. **断点不命中 / 提示 “无调试信息”**：
   - 编译时未加`-g`参数，或加了`-O2/-O3`优化（需改为`-O0`）。
   - `launch.json`中`program`路径指向的不是本地带调试信息的文件。
3. **架构不匹配**：
   - `launch.json`中`set architecture`配置错误（ARM32 填`arm`，ARM64 填`aarch64`）。
   - 交叉编译工具链与开发板架构不匹配（如用 ARM64 工具链编译 ARM32 代码）。
4. **scp 上传失败**：
   - 检查开发板 SSH 服务是否开启（`systemctl status sshd`）。
   - 检查本地用户是否有开发板目标路径的写入权限。

