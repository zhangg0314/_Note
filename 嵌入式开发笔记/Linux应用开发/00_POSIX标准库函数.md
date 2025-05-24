# 命令行解析

## 定义

传统的直接使用`argc` 和 `argv` 的需要手动遍历 `argv`，检查每个参数是否是选项，并处理选项的参数。而使用`getopt` 自动处理选项和参数，分离选项和非选项参数。`getopt` 支持短选项（如 `-h`）和长选项（如 `--help`）。`getopt` 提供了错误处理机制，当用户输入无效选项时，会自动打印错误信息。使用 `getopt` 可以大大简化命令行参数的解析逻辑，减少代码量。

```cpp
#include <unistd.h>
#include <getopt.h>
int getopt(int argc, char * const argv[], const char *optstring);
 
extern char *optarg;
extern int optind, opterr, optopt;


//参数说明：
	//getopt 参数说明：
		argc：通常由 main 函数直接传入，表示参数的数量
		argv：通常也由 main 函数直接传入，表示参数的字符串变量数组
		optstring：一个包含正确的参数选项字符串，用于参数的解析。例如 “abc:”，其中 -a，-b 就表示两个普通选项，-c 表示一个必须有参数的选项，因为它后面有一个冒号。
    //外部变量说明：
		optarg：如果某个选项有参数，这包含当前选项的参数字符串
		optind：argv 的当前索引值
		opterr：正常运行状态下为 0。非零时表示存在无效选项或者缺少选项参数，并输出其错误信息
		optopt：当发现无效选项字符时，即 getopt() 方法返回 ? 字符，optopt 中包含的就是发现的无效选项字符
 //返回值
            成功解析到选项时返回选项字符（如 'h'、'f' 等）。
		   解析完毕返回 -1。
 		   遇到无效选项或缺少参数返回 '?'。
```

## 示例代码

```c++
#include <stdio.h>
#include <unistd.h>
 
int main(int argc, char *argv[]) {
    int o;
    const char *optstring = "abc:"; // 有三个选项-abc，其中c选项后有冒号，所以后面必须有参数
    while ((o = getopt(argc, argv, optstring)) != -1) {
        switch (o) {
            case 'a':
                printf("opt is a, oprarg is: %s\n", optarg);
                break;
            case 'b':
                printf("opt is b, oprarg is: %s\n", optarg);
                break;
            case 'c':
                printf("opt is c, oprarg is: %s\n", optarg);
                break;
            case '?':
                printf("error optopt: %c\n", optopt);
                printf("error opterr: %d\n", opterr);
                break;
        }
    }
    return 0;
}
```

