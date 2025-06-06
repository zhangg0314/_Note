# Linux文件类型

Linux一切皆文件！！Linux文件除了是指一般的文本文件，二进制文件以外，进程间通信的中介、网络套接字，链接文件（快捷方式），目录，设备（网络设备除外）等也算是文件。

> [!IMPORTANT]
>
> - 块设备文件（b）
>
> - 字符设备文件 （c）
>
> - **目录文件（d）**
>
> - 软链接文件（l）
>
> - socket套件字文件（s）
>
> - 管道文件（p）
>
> - **普通文件（r）**

本章主要介绍标准IO与文件IO。其中标准IO是C语言的库，适用于任何操作系统，因此用标准IO的库函数写的程序具有可移植性。其各种函数的应用场景为开发可移植应用时，**主要用普通文件来进行测试**。文件IO是Linux特有的，只适合于Linux，因此用文件IO函数写的程序不具有可移植性。其各种函数的应用场景为开发Linux上的应用，又因为在Linux中目录也是文件，**主要用普通文件和目录文件来进行测试。**

对于标准IO，其针对的对象主要是普通的文件，因为利用标准io函数操作Linux的的设备文件，socket文件仅仅只是在Linux下的应用才能跑，在其他操作系统不一定能跑，所以对于Linux特有的socket文件，设备文件应该用Linux特有的文件IO函数。对于文件IO，虽然这里所介绍的在Linux下里面的各种函数操作对象是普通文件的，但对于设备文件，套接字文件的读写等也是同操作普通文件一样。

<u>总之，标准IO函数针对于所以操作系统共有的普通文本文件或者二进制文件，文件IO函数针对于Linux下特有的设备，管道，套接字等文件，所以为了写出可移植性好的代码，对普通文件应该用标准IO，对Linux特有的文件应该用Linux特有的文件IO。</u>

------

# 	系统调用和库函数

## 系统调用

**系统调用**是用户空间程序与内核空间进行交互的一种方式，即**内核提供给我们的接口**。当用户空间程序需要执行一些内核才能完成的任务（如访问硬件，驱动显卡、创建进程等）时，它会通过系统调用来请求内核的服务。

## 库函数

- 库函数是C或C++等编程语言提供的标准库中的函数，它们通常用于执行常见的编程任务，如字符串操作、数学计算等。库函数可以在用户空间内执行，无需切换到内核态。

- 库函数调用与系统无关，不同的系统，调用库函数时，库函数会调用不同的底层函数实现，因此**可移植性好**。

------

# 标准IO与文件IO

## 文件概念

文件是一组相关数据的有序集合。狭义上指的是磁盘的数据，广义上还要包括设备文件等。

## 文件类型

b、c、d、l、s、p、r(普通文件)

## 两种IO定义

- 标准IO，也称为高级IO，是C语言标准库提供的一种IO操作方式，**统一了各个操作系统的接口调用，可以运行在各个操作系统上**。

- 文件IO，也称为低级IO，**是Linux系统调用提供的一种IO操作方式，独属于Linux操作系统。**

## 两种IO比较

|                | 标准IO（Standard IO）                                  | 文件IO（File IO）                                            |
| -------------- | ------------------------------------------------------ | ------------------------------------------------------------ |
| **定义与特点** | C语言标准库提供的IO操作方式，使用**文件流**和缓冲机制  | Linux系统调用提供的IO操作方式，通过**文件描述符**访问文件，无缓冲机制 |
| **使用场景**   | 适用于**文本文件**的读写操作                           | 适用于**二进制文件**的读写操作，以及对底层设备的访问         |
| **性能特性**   | **具有缓冲机制**，自动处理数据的缓冲和刷新，提高IO效率 | **无缓冲机制**，需要手动管理数据的读写，提供更高的灵活性     |
| **移植性**     | 函数接口来自C语言标准库，与操作系统无关，移植性好      | 函数接口来自Linux内核，与操作系统紧密相关，移植性差          |

# 		标准IO介绍及缓冲区

## 标准I/O介绍

标准I/O由ANSI C标准定义，它是一个标准，主流操作系统上都实现了C库，标准I/O**通过缓冲机制**减少系统调用，实现更高的效率。Linux，一切皆文件，对文件的读写相当于对设备的输入输出，因此就有了标准I/O，即把操作I/O设备当文件来读写。

## 标准I/O的流（FILE）

### FILE介绍	

FILE结构体，别名——流，用来存放文件相关信息的**结构体**，含有文件的读写指针这个成员，表示当前读写到文件的哪个位置了。FILE结构体的定义文件位于`/usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h` 

```c
typedef struct _IO_FILE FILE;
struct _IO_FILE
{
    int _flags;
    
    char *_IO_read_ptr; /*Current read pointer*/
    ......
    char *_IO_write_ptr; /*Current write pointer*/
    ......
    ......
};
//注意事项:
	1)初始打开文件时获得的FILE流指针fp的读写指针为NULL。
    2)当调用输入输出函数时，读写指针如果为NULL，则赋值文件第一个字符位置的地址给读写指针，否则直接进入第三步。
    3)读时读取当前读指针所指位置的字符，然后读指针后移一位，写时，在当前写指针所指位置写入字符，然后写指针后移一位。
```

### 流的读写指针刷新

利用read函数立即读的话，不能读出写入的内容。因为写完之后指针还在后面，即偏移量不为0，所以读取的时候开始位置刚好是刚才写的末尾，所以不可能读取到内容。

```c
#include <stdio.h>
#include <stdlib.h>
int main() {
    FILE *fp;
    // 打开文件用于写入
    fp = fopen("example.txt", "w");
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    // 写入内容
    fputc('H', fp);
    fputc('e', fp);
    fputc('l', fp);
    fputc('l', fp);
    fputc('o', fp);
    --------------------------------------
    // 关闭文件
    fclose(fp);//利用read函数立即读的话，不能读出写入的内容。因为写完之后指针还在后面，即偏移量不为0，所以读取的时候开始位置刚好是刚才写的末尾，所以不可能读取到内容。
    // 重新打开文件用于读取
    fp = fopen("example.txt", "r");
    --------------------------------------
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    // 读取内容
    int c;
    while ((c = fgetc(fp)) != EOF) {
        putchar(c);
    }
    // 关闭文件
    fclose(fp);
    return EXIT_SUCCESS;
}

```

### 流的分类

二进制流、文本流

### 系统自带的FILE

每一个Linux进程默认都会打开`stdin/stdout/stderr`，`stdin/stdout`，它们默认都是行缓冲。

- `stdin` 0(文件描述符)
- `stdout`1
- `stderr`2(**不带缓存**)

## 缓冲区

### 缓冲区作用

输入输出数据并非遇到一个数据就立即输入输出，而是先放到缓冲区，当缓冲区满或者遇到回车换行时才**<u>输入写入磁盘</u>**或者输出，这样就只需要**一次I/O系统调用就可以写入一堆数据**。

**流的缓冲类型**

- **全缓冲**：
  当流的缓冲区无数据或无空间时才执行I/O操作，对于普通**磁盘文件**通常采用全缓冲，即数据先被写入内部缓冲区，待缓冲区满或者显式调用`fflush`函数时，才会将数据写入文件。

- **行缓冲**：
  通常是指遇到换行符'\n'时，标准I/O库会清空缓冲区，将数据写入文件。这通常用于**输入输出终端设备。**

  ```c
  #include <stdio.h>
  #include <unistd.h>
  int main(int argc, char *argv[]){
  	int i = 0;
  	for (i = 0;i < 1024;i++) {
  		printf("a");
  	}
  	while (1) {
  		sleep(1);
  	}
  	return 0;
  }
  //i < 1024时不会输出，这时缓冲区正好满，但不会刷新，要再往里“塞”一个数据才能刷新缓冲区
  //i < 1025，输出1024个a，还一个a留在缓冲区
  ```

- **无缓冲**：
  数据直接写入文件，不经过内部缓冲区。常用的是Linux的**文件IO**函数。

## 标准IO相关函数

> [!NOTE]
> `#define EOF (-1)`

### 文件的打开和关闭

#### 概念

文件的打开就是占用文件资源，文件的关闭就是释放文件资源。

#### 打开文件

```c
FILE* fopen(const char* path,const char* mode) 
    //返回值:成功==流指针，失败==NULL
    //参数说明:
    		1)path:普通文件>>当前路径不需要加目录，其他目录要加上绝对路径
    		2)mode:打开模式>>
```

| char* mode = ? | 含义                                                         |
| :------------: | :----------------------------------------------------------- |
|  "r"    "rb"   | 以只读方式打开文件，文件必须存在                             |
|  "r"    "r+b"  | 以读写方式打开文件，文件必须存在                             |
|  "w"    "wb"   | 以只写方式打开文件，文件不存在则创建，若存在则会清空原有内容 |
| "w+"    "w+b"  | 以读写方式打开文件，其他同"w"                                |
|  "a"    "ab"   | 以只写方式打开文件，文件不存在则创建，若存在则会追加到原有内容后面 |
| "a+"    "a+b"  | 以读写方式打开文件，其他同"a"                                |

#### 关闭文件

```c
int fclose(FILE* stream)
    //返回值:成功==0，失败==EOF
    //参数说明:
		1)stream必须保证不为NULL，否则会报段错误。
    //注意事项:
		1)流关闭时，自动刷新缓冲内容，写入磁盘文件并释放缓冲区。
    	2)当一个程序正常终止时，所有打开的流都会被关闭。
```

### 			文本文件的读写

#### 读取单个字符

```c
int fgetc(FILE* stream)  
int getc(FILE* stream) //宏
int getchar(void)//是阻塞函数
    //返回值:
	1)成功返回读取的字符的ASCII码，失败返回EOF，返回值为int的原因是int范围更大，包括了无符号和有符号字符的范围和EOF
    //注意事项:
	1)getchar从键盘获取字符，等待用户从键盘输入等价fgetc(stdin)。
    2)getc和fgetc的区别是一个是函数一个是宏定义。
    3)字符读写完一个后，FILE结构体的读写指针会后移一位，所以同一程序多次调用fgetc读取的内容不一样，文件是从头开始读的，当文件关闭重新打开又回到开始从头开始读。
	4)EOF（-1）:表示出错，或者到了文件末尾，即返回-1时不一定是出错了，可能是到文件末尾了。
	5)遇到错误：Bad file descriptor 很可能是打开文件的模式错误（只读模式去写，只写模式去读）。
```

#### 写入单个字符

```c
int putchar (int c)//等同于fputc(int c,stdout)，行缓冲
int fputc(int c，FILE* stream)//全缓冲
int putc(int c FILE* stream)
  //返回值:成功-写入的字符的ASCII码，失败-EOF
```

#### 读取行

```c
char* gets(char* s)//不推荐用，未说明大小容易缓冲区溢出，默认从stdin读取

char* fgets(char* s,int size,FILE* stream);
	//返回值:成功-s，出错或者文件末尾-NULL。
    //参数说明:    
		1)size里面空一位存'\0'，故真正只存了size-1个字符.
         2)毕竟读取的是行，而行是以换行符来区别行的，故遇到'\n'或者EOF时结束，如果空间够仍读取'\n'并加上'\0'。
```

#### 写入行

```c
int puts(const char* s)//输出会自动加换行符
             		  //puts不等价于fputs(s,stdout)
int fputs(const char* s,FILE* tream)
      //输出不会加换行符
```

### 		二进制文件读写

#### 概念

二进制文件是指图片，视频等非文本文件。

#### 读写函数

```c
size_t fread(void* p,size_t size,size_t n ,FILE *fp)
   
size_t fwrite(const void* p,size_t size,size_t n,FILE *fp)
    //成功返回对象个数，失败返回-1
    //p存放读写的内容的地址指针，size--对象的大小，n---对象的个数，fp--流指针
    //写时若对象是字符串或者数组，size可以用strlen，如果是其他的如结构体则用sizeof().
```

> [!CAUTION]
> 文件写完后，文件指针指向文件末尾，如果这时候读，读不出来内容。应该先写完关闭文件，再读或者移动读写指针。

### 文件流的刷新和定位

```c
int fflush(FILE* fp)//超重要，特别是while循环不断输出到文件里时一定要刷新缓冲区

long ftell(FILE* fp)
    //此函数返回当前文件流中的读写位置。
    //成功返回流的当前读写位置，失败则返回-1
long fseek(FIFE* fp,long offset,int whence)
    //此函数用于移动文件流中的读写位置。它允许程序以任意顺序访问文件的不同部分，而不是只能从头到尾顺序访问。
    //文件以追加模式a打开时，fseek函数失效
    //成功返回指针位置，失败返回-1
    //whence 参数：
    		SEEK_SET
    		SEEK_CUR
    		SEEK_END
    //offset long 32位，只能打印小于2G文件，
    		负号前移，正号后移
    
void rewind(FILE* stream)
    //此函数将文件流的位置指针设置回文件的开头。
```

### 格式化输入输出

```c
int printf(const char* fmt,...);//输出到屏幕，行缓冲
int fprintf(FILE* stream ,const char* fmt,...)//输出到文件，全缓冲
int sprintf(char* s,const char* fmt,...)//输出到字符串，最后加上'\0'
    
    
int scanf(const char* fmt,...);//从屏幕输入
int fscanf(FILE* stream ,const char* fmt,...)//从文件输入
int sscanf(char* s,const char* fmt,...)//从字符串输入
```

### 标准错误

> [!NOTE]
> 标准出错stderr没有缓冲机制

```c
void perror(const char* s)
    //返回值:void
    //参数说明:
	s：打印用户自定义信息，系统生成的文件错误原因信息或者成功success，会自动换行
    //注意事项:
		能用perror的前提是函数出错时函数会自动set errno。
char* strerror(errno)
    //返回值:返回错误信息字符串的首地址
	// 参数说明:
		errno:系统定义的变量，包含在errno.h头文件中。
```

### 标准IO操作函数总结

| 函数名称 | 调用类型   | 函数功能             | 返回值                    | 读取结束符                                                   | 缓冲类型                      |
| -------- | ---------- | -------------------- | ------------------------- | ------------------------------------------------------------ | ----------------------------- |
| fgetc    | 库函数调用 | 读取一个字符         | 成功-读取的字符，失败-EOF | 读取任意一个字符（包括'\n'）结束                             |                               |
| getchar  | 库函数调用 | 从stdin读取一个字符  | 成功-读取的字符，失败-EOF | 读取任意一个字符（包括'\n'）结束                             |                               |
| fputc    | 库函数调用 | 输出一个字符         | 成功-输出的字符，失败-EOF |                                                              | 全缓冲                        |
| putchar  | 库函数调用 | 输出一个字符到stdout | 成功-输出的字符，失败-EOF |                                                              | 行缓冲                        |
| fgets    | 库函数调用 |                      | 成功-读取的字符，失败-EOF | 遇到''\n'或者读取其他任意size-1个字符结束，'\n'可能会被读取,最后自动加'\0' |                               |
| gets     | 库函数调用 |                      | 成功-读取的字符，失败-EOF | 遇到'\n'结束，'\n'留在缓冲区                                 |                               |
| puts     | 库函数调用 |                      | 成功-读取的字符，失败-EOF |                                                              | 无缓冲（自动加'\n'）          |
| fputs    | 库函数调用 |                      | 成功-读取的字符，失败-EOF |                                                              | stdout-行缓冲，其余文件全缓冲 |
| fread    | 库函数调用 |                      |                           | 读取完任意size个字符结束，不会自动加'\0'                     |                               |
| fwrite   | 库函数调用 |                      |                           |                                                              | stdout-行缓冲，其余文件全缓冲 |

## 标准IO应用案例

使用标准IO每隔1秒向文件`1.txt`写入当前系统时间,行号递增。

[输出日志文件](D:\嵌入式5.0天猫精灵版\应用案例\文件IO\simulate_log.c)

# 文件IO

> [!NOTE]
>
> 文件IO，也叫系统IO，系统调用，常用于二进制文件，函数不能与标准IO函数相互使用，不提供缓冲机制，每次读写都引起系统调用。它没有FILE*，但有文件描述符，它不是一个指针是一个**非负整数**，0-1023，其中文件标识符 0 1 2 分别是`stdin stdout stderr`
>

## 创建普通文件

```shell
vim 文件名
touch 文件名
open()/fopen()函数
```

## 文件IO操作函数

```c
int open(const char* pathname,int flags,mode_t mode)
    //路径，参数，八进制表示的存取权限
    //返回值为int，即文件描述符
    // r		O_RDONLY
    // r+		O_RDWR
    //可以打开设备文件，但不能创建设备文件
int close(int fd)
    //关闭文件后，fd不再代表文件，仅仅是一个数字
    //0成功，-1失败
ssize_t read/write(int fd,void* buf,size_t count)
    //count不超过buf的大小
    //成功返回实际读取的大小，失败返回-1，读空文件不算失败所以读空文件返回0
off_t lseek(int fd,off_t offset,int whence)
    //offset 和whence与fseek一样
```

| flags    | 含义                                              |
| -------- | ------------------------------------------------- |
| O_RDONLY | 只读方式打开文件                                  |
| O_WRONLY | 可写方式打开文件                                  |
| O_RDWR   | 读写方式打开                                      |
| O_CREAT  | 文件不存在创建，且用mode设置其权限                |
| O_EXCL   | 使用O_CREAT时存在，返回错误，用于测试文件是否存在 |
| O_NOCTTY | 使终端（如键盘鼠标）输入输出文件失效              |
| O_TRUNC  | 文件存在时，打开文件则清空原有内容                |
| O_APPEND | 以追加方式打开文件                                |

## 修改文件访问权限

```c
#include <sys/stat.h>
int chmod(const char* path,mode_t mode);
int fchmode(int fd,mode_t mode);
//成功返回0，失败返回EOF
//root和文件所有者才能修改文件的访问权限
//文件权限实际值 = mode - umask
```

## 获取文件属性

### 函数原型

```c
int stat(const char* path,struct stat* buf)
    //如果路径文件是链接文件，此函数找的是目标文件
int lstat(const char* path,struct stat* buf)
    //通过访问struct stat结构体成员可以实现跟ls一样的功能
    //对于链接文件，只找链接，不找链接的来源
	buf->st_mode//
    //低几位用于判断权限
    //高几位用于文件类型
    S_ISREG(buf->st_mode)//本质是位运算取某一位进行与
    S_ISCHR()
    S_ISBLK()
    S_ISDIR()
    S_ISLNK()
    S_ISSOCK()
    S_ISFIFO()
```

### STAT结构体

```c
struct stat {
     dev_t     st_dev;         /* ID of device containing file */
     ino_t     st_ino;         /* Inode number */
     mode_t    st_mode;        /* File type and mode */
     nlink_t   st_nlink;       /* Number of hard links */
     uid_t     st_uid;         /* User ID of owner */
     gid_t     st_gid;         /* Group ID of owner */
     dev_t     st_rdev;        /* Device ID (if special file) */
     off_t     st_size;        /* Total size, in bytes */
     blksize_t st_blksize;     /* Block size for filesystem I/O */
     blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
}
```
