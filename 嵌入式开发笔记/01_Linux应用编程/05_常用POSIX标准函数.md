# POSIX支持多语言

POSIX标准中的函数的程序并不一定要求是 C/C++ 语言。虽然POSIX标准最初是为 C 语言设计的，但许多其他编程语言也提供了对 POSIX功能的访问，通常是通过绑定（bindings）或库来实现的。

1. C/C++• 直接支持：POSIX 标准最初是为 C 语言设计的，因此 C 和 C++ 语言可以直接使用 POSIX 函数。
2. 其他语言简介支持：通常是通过绑定（bindings）或库来实现的。

------

# 时间相关

```c
#include <time.h>
time_t time(NULL)
    //ret -->long int 1970-1-1到现在过的总共的秒数
struct tm* localtime(time_t* tm)
    //返回结构体tm的指针
```

