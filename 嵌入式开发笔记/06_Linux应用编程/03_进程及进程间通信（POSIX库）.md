# 进程的概念

## 1.进程的概念

​	程序是放在磁盘上的一个文件，而进程是程序在分配到资源后运行时的状态。程序只有代码段，数据段（存已初始化数据）和`BSS`段（未初始化数据），而进程还多了栈，堆和进程控制块。

![image-20241005223041259](..\figure\image-20241005223041259.png)

## 2.进程控制块

​	进程控制块，缩写为PCB，是一个包含了进程各种属性的数据结构，有进程`PID`，进程用户，进程状态和优先级，以及**文件描述符数组**等。

## 3.进程类型

### 1.按进程来源分

- **交互进程**
  在shell下启动，可以在前后台运行。这类进程通常与用户直接交互，例如用户在终端中输入命令并执行。
- **批处理进程**
  与终端无关，被提交到一个作业队列顺序执行。这类进程在后台运行，不需要用户的直接干预。例如，定时任务和脚本自动化任务。

  ```bash
   # 通过 cron 或 at 调度的脚本
   crontab -e         # 定时任务
   at now + 1 hour    # 延时任务
   ./build.sh         # 编译脚本批量执行
  ```
- **守护进程**
  与终端无关，即`ctrl+c`无法关闭，一直在后台运行。这类进程通常用于提供系统服务，例如网络服务、日志记录服务等。常见的守护进程有*inetd*、*sshd*等。

### 2.按进程关系分

  - **父进程 / 子进程**

    ```bash
      # bash 是父进程，ls 是子进程
      bash$ ls -l
      # PID  PPID
      # 1234  1000   (ls 的父进程是 bash)
    ```

  - **前台进程 / 后台进程**

    ```shell
    ./app              # 前台进程，占用终端
    ./app &            # 后台进程，终端可继续使用
    nohup ./app &      # 后台进程，终端关闭也不终止
    ```

#  特殊的进程

- PID 0 — idle 进程（调度进程），系统启动时由内核创建，不可杀死
- PID 1 — init/systemd，所有用户进程的祖先
- PID 2 — kthreadd，所有内核线程的父进程

# 进程四种状态

| 状态         |         标志          |            说明             |          例子           |
| ------------ | :-------------------: | :-------------------------: | :---------------------: |
| 运行态 (R)   |   Running/Runnable    | 正在 CPU 执行或在运行队列中 |   正在编译代码的 gcc    |
| 睡眠态 (S)   |  Interruptible Sleep  | 等待事件完成，可被信号唤醒  |   等待用户输入的 vim    |
| 深度睡眠 (D) | Uninterruptible Sleep | 不可被信号唤醒，通常等 I/O  |   等待磁盘 I/O 的进程   |
| 僵尸态 (Z)   |        Zombie         | 已终止但父进程未回收其状态  | 子进程退出后未被 wait() |
| 停止态 (T)   |        Stopped        |         被信号暂停          |    Ctrl+Z 挂起的进程    |

# 僵尸进程

## 1.概念

 	子进程已经执行完毕并退出，但父进程没有调用 wait() / waitpid() 回收其退出状态，导致子进程的
  PCB（进程控制块）残留在内核进程表中，这就是僵尸进程。

## 2.产生过程

```c
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
  int main() {
      pid_t pid = fork();
      if (pid == 0) {
          // 子进程：立刻退出
          printf("子进程 %d 退出\n", getpid());
          exit(0);
      } else {
          // 父进程：不调用 wait()，也不退出，子进程变成僵尸
          sleep(60);  // 父进程干别的事，不管子进程
      }
      return 0;

  }
```

```shell
  子进程 exit() → 内核保留 PCB → 等待父进程 wait() → 回收后才真正消失
                                    ↑
                              如果父进程不做这一步，子进程就一直是僵尸
```

##  3.关键点

​	僵尸进程不占用 CPU，不占用内存，唯一占用的是进程表中的一个条目（PID）。

##   4.危害

  

|     危害     |                             说明                             |
| :----------: | :----------------------------------------------------------: |
|   耗尽 PID   | Linux 默认 PID 最大 32768，大量僵尸会占满进程表，导致无法创建新进程 |
| fork() 失败  |         当进程表满时，fork() 返回 -1，错误码 EAGAIN          |
| 系统管理混乱 |           ps / top 中大量 <defunct>，干扰运维排查            |

  ▎ 注意：僵尸进程不会自动消失，只有父进程调用 wait() 或父进程本身退出（由 init 接管并回收）才会消除。

# 进程的地址空间

- 进程地址空间并不是真实的内存，如果是真实的物理内存那子进程就可以修改父进程的全局变量。
- 既然不是真实的物理内存地址，那么我们在语言层面(C,C++等语言)打印的地址，都是虚拟地址，而真实的物理地址，用户是看不到的，由操作系统统一管理。
- 子进程继承父进程的虚拟地址，所以父子进程访问的数据虽然虚拟地址的地址号相同，但是都被保存映射到了不同的物理内存中。
- 只有物理内存有保存数据的能力。

![image-20241005223014277](..\figure\image-20241005223014277.png)



# 						进程相关命令

## 										1.查看进程

```shell
ps  #默认当前shell下的进程     

ps -e  #linux下所有进程

ps -elf #更详细的信息 |  grep  进程名

top     #查看动态实时的进程信息
top -p pid #查看指定pid的进程动态信息


shift + >或者< #翻页
```

## 										2.进程优先级

```shell
nice -n NI 进程启动命令（比如./a.out)  
#设置优先级并启动进程

renice 优先级 pid  
#重置指定pid进程的优先级，针对于已有且正在运行的进程。
```

## 3.进程前后台切换

- 查看当前shell下的处于后台的进程
  (处于后台的有停止的也有运行的)

  ```shell
  jobs
  ```

- 把进程切换到前台运行 

  ```shell
  fg 数字 
  ```

- 把进程放到后台并停止

  ```shell
  ctrl+z
  ```

- 把进程放到后台运行

  ```shell
  bg 数字 
  ./a.out & 
  ```

# 进程的创建与回收

## 				1.创建子进程

Linux下所有进程除了0号进程，都是子进程，都是由其他进程创建的，且创建进程的函数由**Linux内核实现**。		

```c
#include <unistd.h>
pid_t fork(void)
    //1.创建一个子进程，成功父进程返回子进程pid，子进程返回0，失败返回-1
    //2.通过fork返回值区分父子进程,从而执行不同的代码块
    //3.创建的子进程虽然复制父进程的代码，但执行只执行fork之后的代码，从而避免无限复制，故当子进程执行时，虽然不执行fork，不能通过fork获得pid，但系统会为其分配一个pid，也就是0，也即成功父进程返回子进程pid，子进程返回0
    //4.父子进程执行顺序由操作系统决定

pid_t pid;
pid = fork();
printf("%d\n",(int)pid);
/*
打印结果
5552 父进程
0  子进程
*/
```

## 				2.父子进程的关系

- 子进程继承了父进程的内容,父子进程有独立的地址空间。父子进程变量不共享的，互不影响。
- 若父进程先结束，子进程变成孤儿进程，**被init进程收养子进程变成后台进程，此时ctrl + c无法结束子进程**。
- 若子进程先结束，父进程如果没有及时回收，子进程变成僵尸进程。如果后面父进程结束，则子进程也被收尸了，因为父进程的父进程——shell进程回收了，也会连同把其子进程的子进程一起回收。

## 				3.进程的退出

```c
void exit(int status)
    //status：0-255	
    //@退出进程后，会刷新流缓冲区
    //@main 函数的retrun 0 隐式调用了exit(0)

void _exit(int status)
    //不会刷新流缓冲区
```

## 				4.进程的回收

- 回收自己的子进程

  ```c
  pid_t wait(int* status)
      //成功返回子进程pid以及子进程退出时的状态，失败返回-1
      //@若子进程没有结束，父进程一直阻塞，若有多个子进程，谁先结束，回收谁
      //@status指定保存子进程返回值和结束方式的地址，此外还有一些能判断子进程是否正常结束，判断信号类型等的一些信息。
      //@宏：WEXITSTATUS(status)，对status进行位运算来得到结果。
  ```

- 回收同组子进程或者自己的子进程

  ```c
  pid_t waitpid(pid_t pid,int* status,int option)
      //成功返回子进程pid以及子进程退出时的状态，失败返回-1
      //pid用于指定回收哪个子进程或者任意子进程
     		//@pid == -1,等待任何一个(同组和不同组的)子进程退出，没有任何限制，此时同wait一样
      	//@pid > 0，等待进程id等于pid的子进程结束，其他进程结束也与我无关
      //option用于指定回收方式，0（阻塞的）或WNOHANG(若要回收的进程没结束，不阻塞，直接返回，不回收了)
  waitpid(-1,&status,0) == wait(&status)
  ```

# 		`exec`函数族

## 1.`exec`函数族执行过程

​	当某个程序执行exec(fun1)语句时，进程当前内容被替换成fun1的内容，但<u>**进程号不变**</u>，也就是接下来执行`fun1`。shell进程通过创建子进程，子进程调用exec（`fun1`），而shell作为父进程不受exec影响，所以shell的子进程变成`fun1`.

## 2.`exec`函数族特点

1. 被替换内容的进程号不变。
2. `arg`传递给执行程序的参数列表，参数默认从第0个开始,第0个参数不使用，但不能缺，常用执行程序名补0号位，不然第一个想输入的参数变成0号位就不会使用了，就出错了。

## 3.`exec`函数族函数介绍

`#include <unistd.h>`

- 字符指针存放传入要执行程序的参数

  ```c
  int execl(const char* path,const char* arg,const char* arg,.....,NULL)
      //成功执行程序，失败返回-1
      //path执行程序名字，要包含路径
      //arg传递给执行程序的参数列表，参数默认从第0个开始  
      //最后一个参数为NULL或者(char*)0，固定必写
  int execlp(const char* file,const char*，const char* arg,......,NULL)
      //file执行程序的名字，不用指定路径，系统会在PATH中找
  ```
  
- 字符数组存放传入要执行程序的参数

  ```c
  int execv(const char* path,char* arg[])  
  int execvp(const char* file,char* arg[])
      //就是把const char* arg改成了char* arg[]={"ls","-a","-l","./",NULL}即字符串数组
  ```

- 不会替换程序后面的内容，执行完命令继续执行程序

  ```c
  int system(const char* command)
      //成功返回command的返回值，失败返回EOF
      //@system("ls -a -l ./")
      //@不会替换当前进程system之后的代码，而是等command执行完后，当前进程才继续执行
  ```

# 		守护进程

## 1.守护进程概念

​	守护进程是三种类型进程之一，是Linux中的后台服务进程，是一个生存周期较长的后台进程且独立于终端，不能用fg，bg切换,始终处于后台，独立于终端，周期性执行特定任务或等待处理某些发生的事件。脱离终端，就是为了避免被终端信息所打断，比如摁ctr + c结束一个进程这样的信息，从而成为了一个特殊的孤儿进程，被init进程收养。

## 2.相关概念

- **进程组**
  组id与组长id一样；
- **会话**
  打开一个终端就是打开一个会话，id与组id一样
- **控制终端**
  每一个从终端开始的进程都以该终端为控制终端

## 						3.创建守护进程命令

```shell
1. ./a.out & 
	#切换进程去后台执行，可以通过bg调到前台了执行，并且ctr+c可以停止，即与终端仍有干系，父进程为shell
2. nohup ./a.out & 
	#变成后台进程，与终端无任何干系，不能bg调到前台，父进程为1
```

## 				4.创建守护进程代码步骤

1. 父进程创建子进程，父进程再退出
       父进退出了，子进程的组id会话id仍然和父进程保持一致。

2. 子进程创建新的会话，成为会话组长。

   ```c
   pid_t setsid(void);
   	//调用进程成为一个新的会话组组长，也是一个新的进程组组长，成功返回会话id，失败返回-1
   	//@自己当家做主，sid，pgid都是自己的pid
   
   pid_t getsid(pid_t pid)
       //查看某个进程的会话id
       //成功：返回调用进程的会话ID；失败：-1，设置errno
       //@pid为0表示查看当前进程的会话id
   pid_t getpid(void);
   	//查看当前进程id号
   pid_t getpgid(pid_t pid);
   	//查看当前进程组id
   ```

3. 更改当前工作目录

   这一步不是必须的，但守护进程需要一直运行，因此其工作目录不应该能被卸载，故如果有必要应该把工作目录改成一个比较稳定的工作目录。

   ```c
   chdir(path)
       //path为目标目录
   ```

4. 重置文件权限掩码-----umask(0),并非重置工作目录的掩码，而是程序本身自己创建文件时，程序作为创建者也需要一个掩码，重置的就是这个掩码，即利用程序创建文件的掩码就是该掩码。

   ```shell
   $ umask
   #0022 	(第一个 0 表示是 8 进制，后面的三位数字用 8 进制表示)
   $ umask -S
   #u=rwx,g=rx,o=rx
   umask默认值是0002
   touch一个文件默认权限是0666
   0666-0002 = 0664
   所以：w
   $ touch test.txt
   $ ls -l test.txt
   -rw-rw-r-- 1 name name 0 5月  24 20:49 test.txt
   ```

5. 关闭·`stdin/stdout/stderro`三个文件描述符
           输出关闭了，可以打印到log文件来查看,且后台进程也不接受键盘的输入。

6. 代码实现

   ```c
   #include <stdio.h>
   #include <unistd.h>
   #include <stdlib.h>
   int main(){
       pid_t pid;
       pid = fork();
       if (pid >  0) {
           exit(0);
       }else if (pid < 0){
           perror("fork");
           return 0;
       }else{
           printf("I am a deamon\n");
           sleep(100);
           setsid();
           printf("pid = %d pgid = %d sid = %d\n",getpid(),getpgid(getpid()),getsid(getpid()))
           chdir("/")
           if (umask(0) < 0) {
               perror("umask");
               return 0;
           }
           int i;
           for (i = 0;i < 3;i++) {
               close(i);
           }
       }
   }
   ```

# 	`GDB`调试多进程程序

```shell
start
    #单步开始运行程序，不用手动打断点，系统会自动给你一个一个地往下打断点
set follow-fork-mode child
    #默认跟踪父进程，此命令跟踪子进程
    #脱离gdb的进程会自动不断地运行直至结束，不再是next一步一步的敲一次执行一次了
set detach-on-fork on/off
    #默认是on，off后gdb可以跟踪多个进程，多个进程同时调试。
    #每次调试都要关闭
 info inferiors 
 	#查看进程序号
 inferiors 进程序号(1,2,3....)
 	#切换到给的进程序号的进程下调试。
```

# 	——`IPC`进程间通信——

# 相关`Linux`命令

## 1.查看`IPC`资源

```shell
ipcs [option]

ipcs -l               # 查看系统 IPC 资源限制
ipcs -u               # 查看 IPC 资源使用摘要
ipcs -p               # 显示共享内存段的创建者和最后操作进程ID
ipcs -t               # 显示 IPC 资源的时间信息（最后操作时间等）
ipcs -c               # 显示 IPC 资源的创建者信息

ipcs -a    # 查看所有 IPC 资源（共享内存、消息队列、信号量）
ipcs -m    # 查看共享内存段
ipcs -q    # 查看消息队列
ipcs -s    # 查看信号量集

ipcs -m -i <shmid>    # 查看指定共享内存段的详细信息
ipcs -q -i <msgid>    # 查看指定消息队列的详细信息
ipcs -s -i <semid>    # 查看指定信号量集的详细信息
	
#显示共享内存端 
$ ipcs -m
------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x6403002c 11         linux      666        512        1
```

|   字段   |                     含义                     |
| :------: | :------------------------------------------: |
|  `key`   | `IPC `键值，用于标识资源（可通过`ftok`生成） |
| `shmid`  |     共享内存 ID（进程操作时使用这个 ID）     |
| `owner`  |                 拥有者用户名                 |
| `perms`  |       权限（如 666 表示` rw-rw-rw-`）        |
| `bytes`  |            共享内存段大小（字节）            |
| `nattch` |    **当前连接的进程数**（不是最大连接数）    |
| `status` |      状态（如` dest `表示标记为待删除）      |

## 2.删除`IPC`资源

```shell
ipcrm [option] <shmid>
ipcrm [选项] <ID>
ipcrm <资源类型> <ID>  # 旧式语法，仍支持


ipcrm -m <shmid>    # 删除共享内存段
ipcrm -q <msqid>    # 删除消息队列
ipcrm -s <semid>    # 删除信号量集
ipcrm -M <shmid>    # 强制删除共享内存段（忽略权限）

ipcrm -M <key>      # 通过 key 删除共享内存段
ipcrm -Q <key>      # 通过 key 删除消息队列
ipcrm -S <key>      # 通过 key 删除信号量集

#eg:
	ipcrm -m   11 #删除共享内存
#eg:
	ipcrm -q   msqid #删除消息队列
```

------

# 管道

## 		1.无名管道

### 1.管道本质

​	在内核里开辟一块内存空间用于通信。

### 2.管道特点

- **单工通信**
  具有固定的读端和写端，管道创建时会返回两个文件描述符，分别用于读写管道。

- **使用限制**
  只适用于父子兄弟间通信	
- **解决互斥同步问题**
  管道通过内核中的缓冲区实现数据传递，缓冲区的管理（如读写指针的更新）由操作系统内核控制，自动处理互斥和同步问题，无需用户干预。

### 3.读写特性

- **读管道**

   ```txt
   1.管道有数据，read返回实际读到的个数
   
   2.管道无数据
    1)写端被全部关闭，read返回0
    2)还有写端没被关闭，read阻塞等待让出cpu
   ```

- **写管道**

   ```txt
   1.读端全部关闭，进程异常终止，发送SIGPIPE信号表示管道破裂
   
   2.读端没有全部关闭
   	1)管道满，write阻塞，64k大小的管道。
   	2)管道没满，write返回实际写入个数。
   ```

### 3.函数接口

```c
//功能：创建无名管道
int pipe(int pfd[2]);
/*
	@retval:
		成功返回0
		失败返回-1,并设置errno
	
	@参数返回值:
		pfd[0]为读管道
		pfd[1]为写管道
	
	@注意事项
		进程文件描述符表的0 1 2文件描述符分别是stdin stdout stderror
		3 4 分别是pfd[0]，pfd[1]
	
	@特性
		父进程fork出子进程，子进程也会同父进程一样打开pfd[0],pfd[1]两个文件描述符
		一个进程不能自己写完又读自己写的，即不能又读又写
		可以用于大于2个进程共享，两个子进程可以同时往管道写，而父进程读
*/
```

## 		2.有名管道

### 1.基本概念

- 可以使**非亲缘**进程间进行通信。

- **解决互斥同步问题**
  管道通过内核中的缓冲区实现数据传递，缓冲区的管理（如读写指针的更新）由操作系统内核控制，自动处理互斥和同步问题，无需用户干预。

- 通过路径名来操作，**在文件系统中可见，虽然是文件，但内容存放在内存中而不是磁盘中**，**文件IO**来操作有名管道

- 先进先出

- 不支持leek操作

- **单工读写**

### 2.函数接口

```c
//功能：创建有名管道
int  mkfifo(const char* path,mode_t mode);
/*
	@mode
		管道文件权限
	@path
		要创建的管道文件的路径以及文件名
	@注意事项
		不要建在widows共享目录下，会失败，windows不支持管道文件
         由于管道是单工通信，打开方式只支持只读或者只写
*/
	
open(const char* path,O_RDONLY)
```

### 3.相关特性

1. 一个进程只能用**只读或者只写**方式打开`fifo`文件。
2. 默认打开方式是阻塞打开的，当调用open打开后程序不往下执行，直接被阻塞。
3. 只有读写进程都打开了，读写进程就都不阻塞了（只有写没有读，只有读没有写，默认方式打开open都**不会返回**，都被阻塞）。如果用非阻塞打开，则open立即返回，进程继续往下执行。
4. A process can open a FIFO in nonblocking mode.  In this  case,  opening
   for  read-only succeeds even if no one has opened on the write side yet
   and opening for write-only fails with `ENXIO `(no such device or address)
   unless the other end has already been opened.
   **为了防止管道炸裂，当没有读打开时，写打开会打开失败**
5. **数据完整性**
   当数据写入大小小于`4k`（一块），要么一个不写，要么一次性写入，可以看作写入是原子操作不能写到一半被打断。

------

# 				共享内存

## 1.基本概念

​	在 Linux 系统中，共享内存是一种**高效的进程间通信（`IPC`）机制**，允许多个进程直接访问同一块物理内存区域，从而实现数据的快速交换。

​	相比管道、消息队列等需要通过内核中转的通信方式，共享内存避免了数据在用户空间与内核空间之间的拷贝，因此是性能最优的` IPC `方式之一。

​	涉及到临界资源的访问，由通信进程自己负责实现互斥，可以调用内核提供的`PV`操作。

## 2.内存映射

### 1.基本原理

​	进程在运行时，每个进程都有独立的虚拟地址空间，通过页表映射到物理内存。共享内存的核心思想是：**让多个进程将同一块物理内存区域映射到各自的虚拟地址空间中**。

- 当进程 A 向自己虚拟地址空间中的共享内存区域写入数据时，进程 B 通过自己虚拟地址空间中的对应区域可以直接读取到该数据，无需任何数据拷贝。
- 这种 “直接访问” 特性使得共享内存的通信效率远高于其他` IPC `机制（如管道需要 2 次拷贝：用户→内核→用户）。

### 2.内存映射函数

#### 1.实现原理

​	`mmap`函数就是把磁盘上的文件内存映射到进程的虚拟空间（位于内核中）中，进程访问文件直接访问内存，不需要进程自身调用`read`，`write`等系统调用来完成I/O操作，从而大大提高效率。

#### 2.创建映射

```c
//创建共享内存映射
void *mmap(void *addr，size_t length,int prot,int flags,int fd,off_t offset)
    
    
//@函数返回值：
    /*
    成功返回映射区首地址，失败返回((void*)-1),MAP_FALIED,出错概率高一定要检查返回值的代码避免后续程序执行异常崩溃
    */
    
//@参数说明：
    /*
    addr:
    	要映射的进程的内存首地址，一般为NULL，让操作系统自动选择合适的内存首地址
    	
    length:
    	必须>0，映射内存空间的字节数大小
        指定0，报非法参数错误
        length = lseek(fd,0,SEEK_END),0-end的偏移量正好是文件大小
    
    prot:
    	指定共享内存的访问权限
        PROT_READ  可读
        PROT_WRITE 可写
        PROT_EXEC  可执行
        PROT_NONE  不可访问
        映射区权限 <= 文件打开权限,映射可读可写。文件打开权限必须不小于可读可写
    
    flags:
    	共享内存属性
        MAP_SHARED  共享的
        MAP_PRIVATE 私有的
        私有时，即使写也不会写到磁盘而是写到内存中，自己玩，不会写到磁盘中，故文件打开只需读权限即可，不一定用于进程间通信，而是频繁访问文件，映射该文件来提高效率。
       MAP_ANONYMOUS 匿名映射
       				用于血缘关系的进程间通信
     fd:
       要映射的文件句柄，如果匿名写-1
       被映射的文件大写必须>0，否则会报总线错误
       当写入数据>文件大小，只写入文件大小的量，剩下的丢失
     
     offset:
     	表示从文件的哪个位置开始映射
     	一般为0，从文件头部开始映射。
```

#### 3.释放映射

```c
//释放内存映射
int munmap(void *addr,size_t length);
```

### 3.注意事项

1. 创建映射区过程中，隐含着一次对文件的读操作，将文件内存读到映射区。

2. MAP_PRIVATE私有时，即使写也不会写到磁盘文件而是内存中，自己玩，不会写到磁盘文件中，故文件打开只需读权限也可以往内存中写。而MAP_SHARED共享时，映射区权限 <= 文件打开权限。

3. 内存映射与文件关闭无关，当映射内存建立成功时，关闭文件不会影响映射内存，把文件close了，照样可以通过映射区更改文件。

4. 被映射的文件原本自身的大小必须>0，否则会报总线错误。
   length也必须大于0，否则报非法参数错误。
   offset偏移量必须为0或者`4k`（页大小）的整数倍，否则报非法参数错误。

5. 映射内存大小可以大于或者小于文件大小。
   系统最终分配的可以访问的映射空间为`4k`的整数倍，具体要看文件大小，文件大小小于`4k`，分配`4k`，文件大小大于`4k`小于`8k`，分配`8k`，当访问的地址超出文件大小，虽然修改了内容，但也仅仅是对内存的修改，并不修改磁盘文件的内容。

   ```txt
   比如：
   
   当申请映射的大小>系统实际分配的大小>文件大小
   
   37个字节的文件，可以申请3k映射内存，但系统实际分配4K，因此可以访问地址位于3k~4k之间的内存，但只有对0~37的映射内存空间修改才会引起磁盘文件内容的修改。
   
   内存访问只要在系统分配的内存大小范围内均可，但修改文件只能在文件大小内修改才有效。
   
   要访问的地址超过系统实际分配的内存大小则报总线错误，超过申请的映射空间大小报段错误。
   ```

**共享内存实现进程通信**利用文件映射的内存来进行通信，通信中介是映射内存而不是文件，故内存修改有没有造成文件修改并不关心。因此文件大小哪怕为1但只要不为0都可以，因为目的在于用共享内存通信，不是文件。

### 4.映射类型

1. 文件映射，需要文件。

2. 匿名映射，不需文件，用于血缘关系进程通信。

## 3.实现方式

### 1.`system V`共享内存	

1. **生成key**

   ```c
   key_t ftok(const char* path,int proj_id)
       //功能：生产每个IPC对象唯一的id号，文件节点号拼接proj_id得到id号
       //返回值：成功返回key值，失败返回EOF
       //参数说明：path文件路径,proj_id,用户自定义数字，范围1-255，也可以是单个字符
   ```

2. **创建/打开共享内存**

   ```c
   int shmget(key_t key,int size,int shmflg)
       /*
       @功能：创建一块共享内存
       @返回值：成功返回共享内存id，失败返回EOF
       @参数说明：
       		key:IPC_PRIVATE或者ftok生产
       		size:要创建内存的大小
       		shmflg:PC_CREAT|0666--没有创建，有则打开
       		
   	@IPC_EXCL
   		一个控制标志，用于确保共享内存段的创建具有原子性确保创建一个全新的、不存在的 IPC 对象（如共享内存、消息队列、信号量）。它不是某个长英文短语的简写，EXCL 取自 "Exclusive"（排他/独占）。
   		若共享内存段已存在：
   			shmget会失败（返回-1），并设置错误码EEXIST。
   		若共享内存段不存在：
   			shmget会创建它，并返回新的共享内存 ID（shmid）.
   		IPC_EXCL必须与IPC_CREAT一起使用。
   		若仅指定IPC_EXCL，shmget会直接失败（因为无法获取已存在的内存）。
      
      @避免竞态条件
      		多个进程同时尝试创建同名共享内存时，通过IPC_EXCL可确保只有一个进程成功创建，其他进程会收到明确的 “已存在” 错误，从而避免冲突。
   ```

3. **映射共享内存**
   映射后才能访问内存，读完内存后的东西还在内存里，不像管道一样读完就没了。
   映射后的内存地址是应用程序的虚拟地址，并非真正的物理地址，访问映射内存其实也就是访问内核空间。
   此处映射就是为了把内核空间映射到用户空间中，应用进程才能直接访问。
   
   ```c
   //功能：映射共享内存
   int shmat(int shmid,const void *shmaddr,int shmflg)
       
       /*
       	@返回值
   			成功返回映射后的地址，失败返回（void*）-1
           @参数说明
       		shmid
   				要映射的共享内存id
       		shmaddr
   				映射后的地址，NULL表示由系统自动分配地址
       	     shmflg
       			0表示可读写
       			SHM_RDONLY表示只读
       			
       */
   ```
   
4. **共享内存撤销映射，撤销的是映射关系不是释放内存**

   ```c
    //功能：撤销共享内存，进程结束时自动撤销，撤销让状态数减1，只有当状态数为0也就是没有进程使用内存时才能删除内存
   int shmdt(void *shmaddr)
   /*
   	   @返回值
   			成功返回0，失败返回EOF
          @参数说明：
       		shmaddr：要撤销的地址。
   */
   ```

5. **共享内存控制**

   ```c
   //功能：共享内存控制
   int shmctl(int shmid,int cmd,struct shmid_ds *buf)
    /*
        @返回值	
        	成功返回0，失败返回EOF
        @参数说明
       	shmid
       		要操作的共享内存id
       	cmd
       		要执行的操作
       		IPC_STAT 
       		IPC_SET 
       		IPC_RMID删除
       	buf
       		保持或者设置共享内存属性的地址
    */  
       
   //删除共享内存，队列中的数据也自动删除
   shmctl(shmid,IPC_RMID,NULL)
   ```
   

### 2.`POSIX`共享内存

```c
//创建或打开共享内存对象（类似文件操作）。
int shm_open(const char *name, int oflag, mode_t mode);
//参数：
	name：共享内存名称（必须以/开头，如/my_shm）；
	oflag：打开方式（如O_CREAT | O_RDWR表示创建并读写）；
	mode：权限（如0666）；
//返回值：文件描述符（类似 open ()）。

//设置共享内存的大小,f-file truncate-截断，ftruncate文件截断
int ftruncate(int fd, off_t length);
//ftruncate() 是原子操作，同一文件的多个 ftruncate() 调用不会导致数据混乱，但可能覆盖彼此的结果。
//作用：通过shm_open返回的文件描述符，设置共享内存的大小（必须执行，否则大小为 0）。

//将共享内存映射到进程虚拟地址空间。
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
//参数：
  prot：内存保护（PROT_READ | PROT_WRITE表示读写）；
  flags：MAP_SHARED表示修改对其他进程可见（核心标志）；
//返回值：映射后的虚拟地址。

//解除映射。
int munmap(void *addr, size_t length);

//作用：参数为mmap返回的地址和大小，解除映射。

//删除共享内存对象。
int shm_unlink(const char *name);
//作用：删除共享内存的名称，当所有进程解除映射后，内存被释放。
```

### 3.共享内存的优缺点

#### 1.优点

- **高效性**
  无需数据拷贝，直接访问物理内存，是性能最高的 `IPC` 机制。
- **适合大数据量**
  对高频、大容量的数据交换（如视频流、数据库缓存）非常友好。

#### 2.缺点

- **无同步机制**
  多个进程同时读写时可能导致数据混乱（竞态条件），需配合信号量、互斥锁等同步工具。
- **资源管理复杂**
  System V 共享内存若未显式删除，会一直占用内核资源，可能导致内存泄漏。
- **大小限制**
  系统对共享内存大小有上限（如`/proc/sys/kernel/shmmax`限制 System V 单段最大大小）。

------

# 信号机制

## 1.信号的响应方式

- 缺省方式（默认遵守行为）
- 忽略忽视信号
- 捕捉信号（改变信号所代表的行为）

## 2.信号的产生方式

- 按键产生
- 系统调用产生
- 硬件异常
- 命令行kill产生
- 软件产生（除以0运算）

## 3.常用信号

### 1.可被捕获

|     信号名     |                             含义                             |            默认行为            |                             特点                             |                           产生方式                           |
| :------------: | :----------------------------------------------------------: | :----------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| **`SIGTERM`**  |          请求进程正常终止，是最常用的“优雅终止”信号          |            终止进程            | 可以被捕获、处理或忽略。程序收到后可以执行清理工作（保存数据、关闭文件等） |                           kill命令                           |
|  **`SIGHUP`**  |   最初表示控制终端挂断，现代常用于通知守护进程重新加载配置   |            终止进程            | 许多守护进程（如 `nginx`、`apache`）会捕获它并重新加载配置而不退出 | 1. 关闭终端窗口（会发送给该终端的所有进程） exit<br />2.`kill `命令<br />3.断开伪终端连接（例如 SSH 断开时） |
|  **`SIGINT`**  |                      来自键盘的中断请求                      |            终止进程            |            终止进程、只发给**前台进程组**中的进程            |                   Ctrl + C <br />kill命令                    |
| **`SIGQUIT` ** |    类似 `SIGINT`，但会额外产生 core dump（核心转储文件）     | 终止进程 <br />产生 core dump  |                         可以捕获处理                         |                  Ctrl + \    <br />kill命令                  |
| **`SIGSEGV `** |        Segmentation Violation，进程访问了非法内存地址        | 终止进程 <br /> 产生 core dump |             通常由硬件异常触发，不是用 kill 发送             |                      通常由硬件异常触发                      |
| **`SIGPIPE`**  |        向一个读端已关闭的管道或 socket 写入数据时产生        |            终止进程            |                       常见于网络编程中                       |                       常见于网络编程中                       |
| **`SIGCHLD`**  |      当子进程终止、停止或恢复时，内核向父进程发送此信号      |       忽略（不终止进程）       |               用于异步回收子进程，避免阻塞等待               |                           进程退出                           |
| **`SIGALRM`**  | 由 `alarm()` 或 `setitimer()` 系统调用设置的定时器到期时产生 |            终止进程            |                   可以捕获用于实现超时机制                   |                                                              |

### 2.不可捕获

| **信号名**    |               **含义**               | **默认行为** |                           **特点**                           |                   **产生方式**                    |
| ------------- | :----------------------------------: | :----------: | :----------------------------------------------------------: | :-----------------------------------------------: |
| **`SIGSTOP`** |   暂停进程的执行，直到收到 SIGCONT   |   暂停进程   | **不能被捕获、阻塞或忽略** 进程会进入 TASK_STOPPED 状态 与 `SIGKILL` 是唯二不能被捕获的信号 |            Ctrl + Z    <br />kill命令             |
| **`SIGKILL`** | 立即终止进程，不能被捕获、阻塞或忽略 |   终止进程   | **最暴力**：进程没有机会做清理 <br />不能自定义信号处理函数 <br />保证能杀死进程（除了僵尸进程和内核线程） | kill -9 <PID>   # 强制杀死 <br />kill -KILL <PID> |

**注意**：不能用代码给自己的进程发送 `SIGKILL` 后做一些事情，因为信号处理器根本不会执行。

## **4.信号的发送**

### 1.命令行方式

```shell
#给进程发送信号，因为大部分信号默认操作是终止进程，故取名kill
kill [-signal]  pid

#显示所有信号类型
kill -l

#终止程序
killall [-signal] 程序名
```

|        对比维度        |           **`kill`**            |         **`killall`**          |
| :--------------------: | :-----------------------------: | :----------------------------: |
|      **识别方式**      |      通过**进程ID (PID)**       |        通过**进程名称**        |
| **单个命令可杀进程数** |        1个（除非用循环）        |       所有匹配名称的进程       |
|   **需要先查找PID**    | ✅ 需要（通常配合 `ps`/`pgrep`） |            ❌ 不需要            |
|       **精确度**       |     精确控制（指定哪个PID）     |    模糊（名称相同的一起杀）    |
|       **安全性**       |     更安全（只杀指定的PID）     |    危险（可能误杀同名进程）    |
|   **是否Linux原生**    |         是（POSIX标准）         | Linux 特有（不同系统行为不同） |

### 2.程序代码方式

```c
#include <unistd.h>
#include <signal.h>

//功能:给指定进程发送信号
int kill(pid_t pid,int sig)
/*
    @返回值
    	成功返回0，失败返回EOF
    @参数说明
    	pid接收进程的进程号
    		0  代表同组进程，
    		-1 代表所有进程，
    		>0 发送给指定进程，
    		<-1发送给进程组号为取绝对值的所有组成员
*/

    
//功能:给进程自己发信号
int rais(int sig)
/* 
   等价于kill(getpid(),signo)
*/                    		
```

## 5.发送定时信号

### 1.分类

- alarm() - 简单单次定时器
- ualarm() - 循环/单次微秒定时器
- setitimer() - 高级定时器（最强大）

一个进程只能设定一个定时器

### 2.接口函数

```c
//功能:到达时间发送SIGALRM信号终止进程，且只发一次。
int alarm(unsigned int seconds)

/*
	@返回值：
		成功返回上次定时剩余时间：
			第一次调用，如果调用成功，函数的返回是0。
			第二次调用，返回任何先前计划的报警前剩余的秒数。
		失败返回EOF
	
	@参数：
		 seconds：要定时的秒数	
*/
    
   
    
//功能:循环发送版本的alarm
useconds_t ualarm(useconds_t usecs,useconds_t interval);
/*
	@返回值：
		同alarm，单位是微秒
	@参数：
		第一个参数为第一次产生时间
         第二个参数为间隔产生
*/	
	

 //功能：定时的发送SIGALRM信号，该信号默认终止程序，最灵活的定时器接口，支持多种计时模式、微秒精度和循环触发。
int setitimer(int which,const struct itimerval* new_value,struct itimerval* old_value)
 /*
	@返回值：
		成功：返回 0
		失败：返回 -1（并设置 errno）
	@参数：
		which工作模式：
               		ITIMER_REAL	真实时间---按实际流逝的时间递减，进程被调度或阻塞也会计时
               		ITIMER_VIRTUAL	用户态CPU时间--仅在进程用户态执行时递减（不包含内核态）
               		ITIMER_PROF	用户态+内核态CPU时间--进程执行时递减（包含系统调用时间），用于 profiling
       
       
       new_value - 新的定时器设置
       old_value - 保存旧的定时器设置
			NULL：返回之前定时器的配置
			NULL：不保存旧的配置
       
       struct itimerval
        {
            struct timeval it_interval; // 循环触发间隔（0表示不循环）
            struct timeval it_value; // 第一次触发的时间（0表示取消定时器）
        }
					 
	  struct timeval
	  {
            time_t tv_sec;//seconds
            suseconds_t tv_usec;//microseconds
      }
*/	                            	
```

### 3.示例代码

```c
void handler(int sig) {
    printf("定时器到期！\n");
}

int main() {
    signal(SIGALRM, handler);
    
    unsigned int remaining = alarm(5);  // 5秒后触发
    printf("第一次调用，剩余时间：%u\n", remaining);  // 输出 0
    
    sleep(2);
    remaining = alarm(3);  // 覆盖：重新设为3秒后触发
    printf("第二次调用，上次剩余：%u 秒\n", remaining);  // 输出 3（原来还剩3秒）
    
    pause();  // 等待信号
    return 0;
}
```

## **6.信号的捕捉**

```c
typedef void(*sighandler_t)(int);
sighandler_t signal(int signum,sighandler_t handler);
	//功能:捕捉信号执行自定义函数
	//返回值：成功返回原先的信号处理函数，失败返回SIG_ERR
	//参数：
	   signum：要捕捉的信号
        handler：自定义函数
信号捕捉过程：
        1，定义新的信号执行函数handler
        2，使用signal/sigaction函数把自定义的handle与指定信号相关联
sighandler_t oldfun;
void handler(int sig){
    	//sig：传入handler的参数sig为捕捉到的信号。
        printf("haha");
        signal(SIGINT,oldfun);
 }
int main(){
    oldfun = signal(SIGINT,handler);
    while(1){
        sleep(1);
    }
}
//按一下ctr+c打印haha
//再按一次结束程序




int sigaction(int signum,const struct sigaction *act,struct sigaction* oldact);
	//参数说明：	
	   signum：处理的信号
        act，oldact处理信号的新行为和旧行为
struct sigaction{
    void(*sa_handler)(int);//相当于signal的handler_t
    sigset_t sa_mask;//sigemptyset不屏蔽任何信号,可以看成其sigaction内部实现了sigpromask(SIG_BLOCK,&sa_mask,NULL);
    int sa_flgs;//sa_flags 指定一组修改信号行为的标志。 它由以下零个或多个的按位或组成，0表示默认行为，不设置    
    
}
```

## 		**7.信号的阻塞**

**概念：信号的阻塞是一个开关动作，指的是阻止信号被处理，且是在处理其他信号时阻塞，并不是时时刻刻都被阻塞，但不是阻止信号产生，阻塞信号，他收到了信号只是没处理，延时过了一段时间后解除阻塞，就会立即执行该信号，并不会因为阻塞就不会再执行了。**

**信号状态：**

**信号递达：实际信号执行的处理过程**

**信号未决：从产生到递达之间的状态**

```c
sigset_t set;//自定义信号集，实质是一个一个的bit位组成的一个bit位数组，数组下标即信号编号signo
sigemptyset(sigset_t *set);//清空信号集
sigfillset(sigset_t *set);//全部置1
sigaddset(sigset_t *set,int signum);
sigdelset(sigset_t *set,int signum);
sigismember(const sigset_t *set,int signum);//判断一个信号是否在集合中


int sigprocmask(int how,const sigset_t *restrict set,sigset_t *restrict oset)
	//功能：设定对信号集内的信号的处理方式（阻塞或不阻塞）
    //返回值：成功0，失败-1
    //参数：
    	    how，
    			SIG_BLOCK把参数set中的信号添加到信号屏蔽字中,
			    SIG_UNBLOCK把参数set中的信号从信号屏蔽字中删除，
                 SIG_SETMASK把信号屏蔽字的每一位设置为参数set中的每一位
    		set，信号集
    		oset，默认为NULL
```

## **8.信号驱动**

```c
int pause(void)
    //功能：让进程一直阻塞直到被信号中断，停止阻塞
    //返回值：只有-1
	//函数说明：
    	1.如果接收到的信号默认处理是终止进程，则进程终止，pause函数没有机会返回
    	2.如果信号的默认处理动作是忽略，则进程继续处于挂起阻塞状态，pause函数不返回
    	3.如果信号处理动作是捕捉，则调用完信号处理函数之后，pause返回-1，进程继续往下执行
    	4.如果信号被屏蔽，pause不能被唤醒
    while(1){
        pause();
        task();
        sleep(1);
    }//发一次信号唤醒一次pause，执行一次task，即用信号驱动执行task
注意！！
    如果先pause阻塞再接收信号处理，则pause不再阻塞
    如果先接收信号处理，再pause，则pause继续阻塞
    sigprocmask(SIG_UNBLOCK,&set,NULL);
	pause();
	取消信号屏蔽后，会立刻执行信号处理函数，再继续执行pause时pause收不到信号故继续阻塞！！
int sigsuspend(const sigset_t *sigmask);
	//功能说明：将进程的屏蔽字替换为由参数sigmask给出的信号集，即屏蔽字bit位和sigmask一样，然后挂起进程的执行(同挂起进程即执行pause)
	如果sigmask 全为0，表示不屏蔽任何信号，此时等价于将		
        sigprocmask(SIG_UNBLOCK,&set,NULL);
		pause();合并为一个原子操作，期间不能被打断！！！
	//参数说明：sigmask希望屏蔽的信号
```

# **消息队列**

## 1.相关概念

​	消息队列（Message Queue）是一种进程间通信（Inter-Process Communication，`IPC`）机制，它允许不同进程之间通过在队列中发送和接收消息来交换数据。

​	消息队列可以看作是一个先进先出（FIFO）的数据结构，其中每个消息都包含一定格式和长度的数据。

​	`System V`消息队列是UNIX系统V（一个早期的UNIX系统标准）的一部分，它将消息队列的数据结构存储在内核中。这意味着消息队列的操作需要通过系统调用来进行，因为它们涉及到内核空间和用户空间之间的数据交换。

## 2.相关特性

- **可靠有序**
  当一个消息被取出（接收）后，它就会从队列中移除，不会留在队列中。这种机制确保了消息的传递是可靠和有序的，即每个消息只会被消费一次，并且按照它们被放入队列的顺序来处理。

- **同步有序**
  消息队列的自然同步特性，消息队列本身是按照先进先出（FIFO）的顺序处理消息的，这意味着消息的发送和接收是有序的。这种有序性在一定程度上提供了一种自然的同步。在某些高级的消息队列系统中，消息的接收可能需要确认。只有当接收者确认消息已被处理后，消息才会从队列中移除。这也是一种同步机制。

- **原子性**

  消息处理的完整性即原子性：确保消息的发送和接收操作是原子性的，即在消息被完全发送或接收之前，不会被其他进程的操作打断。大多数消息队列实现已经保证了这一点，但具体情况还需查看具体的系统文档。

## 3.发送端（System V）

### 1.申请key

- **硬编码**
  可以直接为消息队列指定一个 `key_t `（本质就是int）类型的键值，这通常是通过硬编码一个整数值来实现的。这种方法简单，但不够灵活，因为硬编码的键值可能在不同的系统或环境中重复。
- **`ftok`函数**
  是根据给定的路径名和项目ID来生成一个唯一的key_t键值。这是最常用的方法，因为它可以保证在同一个系统上，对于同一个路径名和项目ID，生成的键值是唯一的。
- **私有队列键值创建**
  在某些情况下，系统可能会自动为新创建的消息队列分配一个键值。这通常发生在使用 `msgget`函数创建消息队列时，没有指定键值，或者指定了`IPC_PRIVATE`作为键值。

### 2.打开/创建消息队列

```c
int msgget(key_t key,int msgflg)
    //功能：创建/打开消息队列
    //返回值：成功返回消息队列id，失败返回EOF
    //参数说明：
    		key：IPC_PRIVATE或者ftok生产,使用IPC_PRIVATE作为键值时，系统会自动生成一个唯一的消息队列标识符，这个消息队列只能被创建它的进程访问，其他进程无法访问。
    		msgflg：IPC_CREAT|0666--没有创建，有则打开
```

### 3.发送消息

```c
int msgsnd(int msgid,const void* msgp,size_t size,int msgflg);	//原子操作
/****@return****
//成功返回0，失败返回-1

/****@parama****
**msgid  消息队列id,由msgget获取
**msgp   消息缓冲区地址
**size   正文长度
**msgflg 标志位 0 或IPC_NOWAIT.0:当消息队列满时，msgsnd会阻塞，直到消息能写进，IPC_NOWAIT则不等待立即返回。
*/      
//消息格式
typedef struct{
    long msg_type;
    char buf[128];//不一定是char buf，可以是结构体，反正第一个成员必须固定，其余成员可以增加成员，也可以改变成员。
}msgT;
int MSGLEN = sizeof(msgT)-sizeof(long)
int msgsnd(msgid,&msgT,MSGLEN,0)
    //msg_type消息类型必须指定，但发送消息时的长度不包含msg_type的长度，而POSIX方式的长度包含msg_type即一个long类型的大小
    //msg_type取值必须>0
```

## 3.发送端实现（POSIX）

### 1.使用流程

```c
//创建或打开消息队列 ---> 向队列写入消息 ---> 从队列读取一条消息 ---> 断开进程与队列间的关联关系 ---> 删除队列
mq_open()--->mq_send()--->mq_receive()--->mq_close()--->mq_unlink()
```

### 2.打开或创建消息队列

```c
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
/*
 *  @Description: 创建一个新的消息队列或打开一个既有消息队列
 *  @Para       : const char * name         消息队列的名称
 *  int oflag                 位掩码  为O_CREAT则创建一个队列 非O_CREAT为打开一个既有队列
 *  mode_t mode               （创建队列时）位掩码 可以设置队列的文件权限
 *  struct mq_attr * attr     （创建队列时）设置消息队列的特性
 *  @return     : 成功返回消息队列描述符，失败返回-1
 *  */
 mqd_t mq_open(const char * name,  int oflag [, mode_t mode, struct mq_attr * attr]);
//例：创建一个消息队列
if(mq_open("/myqueue", O_CREAT | O_EXCL | O_RDWR, 0644, NULL) == -1)
    printf("queue creat failed!\r\n");


//@注意：
1).消息队列的名称参数name，必须以斜线开头，后面跟着一个或多个非斜线字符的名字，如 “/myqueue”, 名称的最长字符数为 NAME_MAX(255) - 4;
2).当int oflag = O_CREAT | O_EXCL 时，如果消息队列不存在则创建，如果存在，则函数失败返回。可以设置如下特性：
								O_CREAT //队列不存在时创建队列
								O_EXCL //与O_CREAT一起排它地创建队列
								O_RDONLY //只读打开消息队列
								O_WRONLY //只写打开消息队列
								O_RDWR //读写打开消息队列
								O_NONBLOCK//以非阻塞模式打开，此时如果mq_send(),mq_receive()不能在不阻塞的情况下执行，											  则会立即返回EAGAIN错误
3)如果mq_open()用于打开一个既有消息队列，则仅适用前两个参数即可。如果是创建新消息队列，则可以通过mode_t mode设置新创建的消息队列的文件权限[Owner Group Other, Read Write Execute等]，通过 struct mq_attr * attr可以设置消息队列的属性,如果设为NULL则为默认属性。
4)当打开或者创建消息队列时，会自动建立进程与消息队列间的关联关系，可以通过mq_close()断开该关联关系；
    
    
//消息队列的特性struct mq_attr
    struct mq_attr{
    long mq_flags;      //0 或者 O_NONBLOCK  创建时，创建后均可修改
    long mq_maxmsg;     //创建的消息队列所能添加消息的数量上限，其取值必须大于零，仅创建的时候可以设置。
    long mq_msgsize;    //消息队列中每条消息的大小上限，其取值必须大于零，仅创建的时候可以设置。
    long mq_curmsgs;    //消息队列中，消息的数量，只读
};
//消息队列的属性参数仅有 mq_flags 可以在创建后设置。
```

### 3.获取与设置消息队列属性

```c
#include <mqueue.h>
/*
 *  @Description: 获取指定消息队列的属性
 *  @Para       : mqd_t mqdes         消息队列描述符
 *                struct mq_attr * attr   返回属性结构的存储地址
 *  @return     : 成功返回0，失败返回-1
**/
int mq_getattr(mqd_t mqdes, struct mq_attr * attr);

/*
 *  @Description: 设置指定消息队列属性
 *  @Para       : mqd_t mqdes         消息队列描述符
 *                struct mq_attr * newattr   设置消息队列的结构
 *                struct mq_attr * oldattr   如果不为NULL，则此结构将会返回设置前的消息队列属性，与mq_getattr()执行的任务相同。
 *  @return     : 成功返回0，失败返回-1
**/
int mq_setattr(mqd_t mqdes, const struct mq_attr * newattr, struct mq_attr * oldattr);
```

### 4.发送接收消息

```c
#include <mqueue.h>
#include <time.h>
/*
 *  @Description: 发送消息到指定消息队列中
 *  @Para       : mqd_t mqdes            消息队列描述符
 *                const char * msg_ptr   发送的消息存储区首地址
 *                size_t msg_len         发送消息的长度(包括消息类型）
 *                unsigned int msg_prio  发送消息的优先级，0为最低优先级
 *  @return     : 成功返回0，失败返回-1
**/
int mq_send(mqd_t mqdes, const char * msg_ptr, size_t msg_len, unsigned int msg_prio);

/*
 *  @Description: 发送消息到指定消息队列中，如果阻塞发送，到达超时时间则返回ETIMEDOUT错误
 *  @Para       : mqd_t mqdes            消息队列描述符
 *                const char * msg_ptr   发送的消息存储区首地址
 *                size_t msg_len         发送消息的长度（包括消息类型）
 *                unsigned int msg_prio  发送消息的优先级，0为最低优先级
 *                const struct timespec * abs_timeout   到达该绝对时间还未完成发送，则返回错误
 *  @return     : 成功返回0，失败返回-1
**/
int mq_timedsend(mqd_t mqdes, const char * msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec * abs_timeout);

/*
 *  @Description: 从指定消息队列接收消息
 *  @Para       : mqd_t mqdes            消息队列描述符
 *                const char * msg_ptr   接收消息存储区首地址
 *                size_t msg_len         接收消息的长度
 *                unsigned int * msg_prio  返回接收到的消息的优先级
 *  @return     : 成功返回接收到的字节数，失败返回-1
**/
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int * msg_prio);

/*
 *  @Description: 从指定消息队列接收消息，如果阻塞接收，到达超时时间则返回ETIMEDOUT错误
 *  @Para       : mqd_t mqdes            消息队列描述符
 *                const char * msg_ptr   接收消息存储区首地址
 *                size_t msg_len         接收消息的长度
 *                unsigned int * msg_prio  返回接收到的消息的优先级
 *                const struct timespec * abs_timeout   到达该绝对时间还未完成接收，则返回错误
 *  @return     : 成功返回接收到的字节数，失败返回-1
**/
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int * msg_prio, const struct timespec * abs_timeout);
```

### 5.关闭和删除消息队列

```c
#include <mqueue.h>
/*
 *  @Description: 断开当前进程与消息队列的关联关系
 *  @Para       : mqd_t mqdes            消息队列描述符
 *  @return     : 成功返回0，失败返回-1
**/
int mq_close(mqd_t mqdes);

/*
 *  @Description: 将消息队列name标记为：当所有进程与该消息队列断开关联关系后销毁该队列。
 *  @Para       : const char * name       要删除的消息队列名称
 *  @return     : 成功返回0，失败返回-1
**/
int mq_unlink(const char * name);


//进程在使用完消息队列后，应该使用mq_close()断开关联关系，释放描述符，防止出现消息队列耗尽描述符的情况。
//mq_unlink()仅做标记，是否马上删除消息队列决定于是否仍有进程关联消息队列
```

## 4.接收端实现（System V）

### 1.申请key，打开/创建消息队列

同发送端

### 2.接收消息

```c
int msgrcv(int msgid,void* msgp,size_t size,long msgtype,int msgflg);//（原子操作）
	//msgid:消息队列id
	//msgp：消息缓冲区地址
	//size：指定接收的消息长度
	//msgtype：指定接收的消息类型，与msg_type不一样。
	   =0：接收第一条为任意类型的消息
       >0:接收第一条为msg_type类型=msgtype类型的消息
       <0:接收第一条类型处于[0,|msgtype|]范围的第一条消息
    //msgflag：
            0：阻塞式接收消息
            IPC_NOWAIT:没消息立即返回，并返回错误码
 //@注意：
      消息队列拿几条消息少几条消息，不拿则留在队列里
      先进先出的要符号消息类型匹配，故是有选择性的先进先出。
```

## 5.删除消息队列

```c
int msgctl(int msgid,int cmd,struct msgid_ds *buf)
    //功能：消息队列控制
    //返回值：成功返回0，失败返回EOF
    //参数说明：
    		shmid：要操作的消息队列id
    		cmd：要执行的操作，
    					IPC_STAT 从内核中拷贝结构体内容给buf
    					IPC_SET  设置buf的内容到内核中
    					IPC_RMID删除
    		buf：保持或者设置消息队列属性的地址
```

# 信号量(灯)

## 1.基本概念

- 在 Linux 系统里，信号量（Semaphore）是进程间通信（`IPC`）的重要手段，主要用于实现资源的同步与互斥访问。
- 信号量本质上是一个计数器，它的作用是控制多个进程或线程对共享资源的访问。
- 信号量(semaphore)又名信号灯，信号量代表某一类资源，其值表示系统中该资源的数量。
- 信号量是一个受保护的变量，只能通过三种操作来访问即`初始化，P操作，V操作`，主要用于进程或者线程间的同步主要。

## 2.按计数个数分类

- **二值信号量**
  其值只有 0 和 1，功能类似于互斥锁，用于实现对资源的互斥访问。
- **计数信号量**
  它的值可以是任意的非负整数，能够允许多个进程或线程同时访问共享资源。

## 3.信号量的种类

- `posix`有名信号灯（进程间）

- `posix `无名信号灯（线程间）

- `system V` 信号灯（进程间）

## 4.实现方式

### 1.`POSIX `线程信号量（Thread Semaphores）

这是基于线程的轻量级实现，适用于同一进程内的线程同步，用的是无名信号灯，使用步骤如下：

```c
#include <semaphore.h>
//功能：创建并初始化信号量
int sem_init(sem_t *sem,int pshared,unsigned int value);
/*
	@pshared
		表示进程或者线程间是否共享该信号量，一般为0不共享，即由初始化这个信号量的进程使用。
		0：线程间共享（同一进程内）
		非0：进程间共享（需要将 sem_t 放在共享内存区域，如 mmap 的共享映射或 shm_open）
	@sem
		指针，但是是要指向了某一块地址的指针，故应该传&a.
*/


// ❌ 错误：pshared=1 但放在普通全局变量
sem_t sem;
sem_init(&sem, 1, 1);  // 无效！父子进程不共享该变量内存

// ✅ 正确：进程间使用需要共享内存
int fd = shm_open("/mysem", O_CREAT | O_RDWR, 0666);
ftruncate(fd, sizeof(sem_t));
sem_t *sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, 
                  MAP_SHARED, fd, 0);
sem_init(sem, 1, 1);


sem_t semaphore;
sem_init(&semaphore, 0, 1); 
// 等待信号量（P操作）
sem_wait(&semaphore);
// 释放信号量（V操作）
sem_post(&semaphore);
// 销毁信号量
sem_destroy(&semaphore);
```

|      函数       |       作用       |             备注              |
| :-------------: | :--------------: | :---------------------------: |
|   `sem_init`    | 初始化匿名信号量 |    仅用于线程间或相关进程     |
|   `sem_wait`    |  P操作（阻塞）   |        信号量为0时阻塞        |
|  `sem_trywait`  |   非阻塞P操作    | 立即返回，成功0，失败`EAGAIN` |
| `sem_timedwait` |     限时等待     |     超时返回 `ETIMEDOUT`      |
|   `sem_post`    |  V操作（唤醒）   |      可唤醒等待线程/进程      |
|  `sem_destroy`  |    销毁信号量    |      确保没有线程在等待       |
| `sem_getvalue`  |    获取当前值    |   调试用（值可能瞬间过时）    |

### 2.`POSIX` 命名信号量（Named Semaphores）

这种信号量以文件形式存在，主要用于不同进程间的同步。

> [!CAUTION]
>
> 第一次open创建文件，并赋初值，若退出程序，再次运行程序，再次open，由于文件已经存在则所有信号量值不管怎么设置都默认为0，解决办法就是程序退出时应该删除信号量`sem_unlink()`
#### 1.`sem_open`完整用法

```c
#include <fcntl.h>      // O_CREAT, O_EXCL
#include <sys/stat.h>   // 文件权限
#include <semaphore.h>
//功能：创建命名信号量
sem_t *sem_open(const char* name,int oflag,mode_t mode,unsigned int value);
/*
	@name
		信号文件名，通过文件名来让两个程序虽然各自定义了锁，但文件名相同则各自定义的锁也就一样，创建的文件文件放在/dev/shm目录。
	@oflag
		打开方式常用O_CREAT
	@mode：文件权限，常用0666
	@value：信号量初始值

*/

//关闭
int sem_close(sem_t *sem)
// 关闭当前引用,当所有进程都关闭该信号量后，内核对象才会真正销毁
int sem_unlink(const char* name)

 
#include <fcntl.h>
#include <semaphore.h>

// 创建/打开命名信号量
sem_t *sem = sem_open("/mysem", O_CREAT, 0666, 1);
// 仅打开已存在的信号量（不创建）
sem_t *sem = sem_open("/mysem", 0);


// 使用
sem_wait(sem);
// ... 临界区 ...
sem_post(sem);

// 关闭并删除信号量
sem_close(sem);
sem_unlink("/my_semaphore");

// 关闭（不删除信号量）
sem_close(sem);

// 删除信号量（所有进程close后真正销毁）
sem_unlink("/mysem");



// ❌ 错误：name 格式不对
sem_open("mysem", O_CREAT, 0666, 1);    // 缺少开头斜杠
sem_open("/tmp/mysem", O_CREAT, 0666, 1); // 不能有多级路径

// ✅ 正确：以单斜杠开头
sem_open("/mysem", O_CREAT, 0666, 1);
```

#### 2.`oflag` 常见组合

|   特性   | `sem_init` (匿名) |    `sem_open` (命名)     |
| :------: | :---------------: | :----------------------: |
| 适用场景 | 线程间、相关进程  |        任意进程间        |
| 存储位置 |   用户指定内存    | 内核持久化（`/dev/shm`） |
| 清理方式 |   `sem_destroy`   |       `sem_unlink`       |
| 多次打开 |        否         |      是（通过名字）      |

### 3.`System V `信号量

这是一种比较传统的 `IPC `机制，支持信号量集，使用起来相对复杂。

```c
//创建/打开信号灯
int semget(key_t key,int nsems,int semflg);
/*
	@参数
		key:....
    @nsems
    	信号灯集包含的信号的个数
    @semflg
		访问权限，通常为IPC_CREAT |0666
    @返回值
    	成功semid，失败-1
*/

int semctl(int semid,int semnum,int cmd,[union semun*])
/*
    @semid
    	信号灯集ID
    @semnum
    	要操作的集合中的信号的编号，从0开始
    @cmd
    	GETVAL：获取信号灯的值，返回值是获得值
    	SETVAL：设置信号灯的值，需要用到第四个参数unino结构体
    	IPC_RMID：从系统中删除信号灯集
    @返回值
    	成功0，失败-1
*/
    
//读和写：信号灯个数就为2
#define SEM_READ 0
#define SEM_WRITE 1
    
    
//信号灯初始化赋值             
union semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* array for GETALL, SETALL */
    /* Linux specific part: */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};
struct sembuf{
       short sem_num//信号灯编号
       short sem_op// 1.V
            	   //-1.P
       short sem_flg://0阻塞，IPC_NOWAIT,SEM_UNDO
}

//信号灯赋值
semctl(semid,0,SETVAL,semun)
//信号灯删除
semctl(semid,0,IPC_RMID)
    
    
/* Flags for `semop'.  */
#define SEM_UNDO	0x1000		/* undo the operation on exit */
void sem_p(int semid,int semindex){
    struct sembuf sbuf;
    sbuf.sem_num = semindex;
    sbuf.sem_op = -1;
    sbuf.sem_flg = 0;
    semop(semid,&sembuf,1);
}
void sem_v(){
    struct sembuf sbuf;
    sbuf.sem_num = semindex;
    sbuf.sem_op = 1;
    sbuf.sem_flg = 0;
    int nops = 1;//要操作的信号灯个数
    semop(semid,&sembuf,nops);
}
```

## **4.结构体扩展**

 `struct semid_ds *buf`; 

```c
/* Data structure describing a set of semaphores.  */
struct semid_ds
{
  struct ipc_perm sem_perm;		//描述信号量集的权限信息,包含：uid 和 gid：所有者和组的 ID。
													   //mode：访问权限位（如 0666）。
													  //key：创建信号量集时使用的键值（通过 ftok() 生成）。
                                                            //seq：序列号，用于区分具有相同键的不同实例。
  __time_t sem_otime;			//最后一次调用 semop()（执行 P/V 操作）的时间戳,若 sem_otime 为 0，表示信号量集刚创建但尚未被任何进程操作过。
  __syscall_ulong_t __glibc_reserved1;
  __time_t sem_ctime;	//最后一次通过 semctl() 修改信号量集的时间戳（如调用 SETVAL 初始化值）
  __syscall_ulong_t __glibc_reserved2;
  __syscall_ulong_t sem_nsems;		/* number of semaphores in set */
  __syscall_ulong_t __glibc_reserved3;
  __syscall_ulong_t __glibc_reserved4;
};
```

## 5.典型应用场景

- **互斥访问**
  借助二值信号量来保证同一时间只有一个进程或线程能够访问共享资源。
- **资源计数**
  利用计数信号量对可用资源的数量进行管理，比如数据库连接池。
- **生产者 - 消费者模型**
  可以使用多个信号量分别对缓冲区的空槽和数据项进行计数。

## 6.与互斥锁的差异

- **信号量**
  支持多个资源实例，适用于进程间同步，并且可以实现复杂的同步模式。
- **互斥锁**
  只允许一个持有者，主要用于线程间同步，而且通常具有更高的性能。

# 套接字

见网络课程

# 各种通信方式对比

## 1.管道（Pipe）

- **特点**
  半双工通信，数据只能单向流动，通常用于具有亲缘关系的进程（如父子进程）之间的通信。
- **效率**
  相对较低，因为数据需要在内核中缓存，且通信方式较为简单。
- **使用场景**
  适用于简单的、数据量不大的父子进程间通信。

## 2.命名管道（Named Pipe）

- **特点**
  与管道类似，但具有唯一的名称，可以在文件系统中进行访问，支持无亲缘关系的进程间通信。
- **效率**
  与管道相似，但使用上更为灵活。
- **使用场景**
  适用于需要在不同进程间传递数据的场景，尤其是当这些进程没有直接的亲缘关系时。

**管道缺陷：只能传送简单字节流，且缓冲区空间有限。**

## 3. 信号（Signal）

- **特点**
  异步通信方式，用于通知接收进程某个事件已经发生。
- **效率高**
  因为信号传递的是事件而非数据，处理速度快。
- **使用场景**
  适用于需要快速响应异步事件的场景，如处理中断、终止进程等。
- **优缺点**
  简单灵活，但信号个数有限且传递的信息量也少。

## 4.共享内存（Shared Memory）

- **特点**
  允许多个进程访问同一块物理内存，实现数据共享。

- **效率非常高**
  因为数据直接在内核的内存中访问，无需像驱动开发那样还要用户空间数据拷贝到内核空间，且减少系统调用次数从而降低开销。

- **使用场景**

  适用于需要频繁交换大量数据的场景，但需要配合其他同步机制（如信号量）使用。

## 5. 信号量（Semaphore）

- **特点**
  主要用于进程间同步和互斥，控制对共享资源的访问。
- **效率**
  依赖于具体的实现方式，但通常作为同步机制的一部分，不直接传输数据。
- **使用场景**
  与共享内存等机制配合使用，确保多个进程在访问共享资源时的同步和互斥。

## 6.消息队列（Message Queue）

- **特点**
  在内核中创建消息队列，允许进程之间传递消息。消息可以是结构化的数据，支持双向通信。

- **效率适中**
  避免了信号传递信息量少和管道只能承载无格式字节流的缺点，但因为队列的存在，通信效率相对低一些。

- **使用场景**

  适用于需要传递结构化数据或复杂消息的进程间通信场景，已经需要保持同步的操作流程的比如一发一收的场景。

## 7. 套接字（Socket）

- **特点**
  不仅可用于本地主机上的进程间通信，还可用于跨网络的进程间通信。
- **效率**
  依赖于网络条件和通信协议，但提供了极高的灵活性和可扩展性。
- **使用场景**
  适用于分布式系统、网络通信等需要跨进程、跨主机通信的场景。