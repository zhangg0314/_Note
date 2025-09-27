# 	GCC编译工具链

## 1.GCC概念

`GNU C Complie`编译器单独指C编译器，但现在多指GNU Complie Collection ，是一个工具集合。

## 2.GCC编译过程

 ```bash
 gcc -E	#(预处理)    .c ---> .i
 gcc -S	#(编译:检查语法错误) .i ---> .s(汇编文件)
 gcc -c	#(汇编)     .s-->.o(二进制文件)
 gcc		#(链接)         各种.o-->可执行文件.elf
 
 #所有的.c文件都有经过预处理，编译，汇编三个步骤。
 #这三个步骤独立，不依赖不依靠其他文件的功能，库函数，即不是说别的函数调用了stdio库，我就可以不调了。
 #最终独自生成.o文件，然后所有.o文件链接形成一个可执行文件。
 
 
 -a为可执行程序，b为动态库，c为动态库
 -b编译时要链接c
 -a编译时既要链接b也要链接c #不依赖不依靠其他文件的功能，库函数
 -b链接c只是为了编译通过不报错而正确生成b.so的文件，但b.so文件并未包含c的代码，因此编译a时仍然需要链接c
 ```

## 3.常用GCC命令

```shell
gcc -I	#（大写i） 指定头文件的路径：指定头文件

gcc -l	#（小写L） 库名 ：链接库
gcc tets.c -lpthread

gcc -L 	#指定库的路径

gcc -O/O2  #优化代码，同volatile里优化意思一样

gcc -g     #进行调试---->gdb ./a.out
```

## 		4.条件编译

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

# arm-linux-xx

## 1.链接.o文件

```bash
arm-linux-ld -Txx.lds -o a.out main.o math.o
#xx.lds,链接脚本
```

## 2.生成bin文件

```
arm-linux-objcopy -o binary a.out a.bin
```



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

