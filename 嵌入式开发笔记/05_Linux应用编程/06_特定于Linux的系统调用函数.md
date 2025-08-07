# sysinfo

## 1.功能说明

`sysinfo` 是一个用于获取系统信息的函数，在 Linux 系统中，它定义在 `<sys/sysinfo.h>` 头文件中，函数原型如下：

```c
int sysinfo(struct sysinfo *info);
```

`sysinfo` 函数的主要作用是获取当前系统的整体信息，包括内存使用情况、系统运行时间、进程数量等。调用成功时返回 0，失败时返回 -1 并设置 `errno`。

## 2.结构体 `struct sysinfo` 详解

该结构体用于存储 `sysinfo` 函数获取的系统信息，主要成员包括（不同系统可能略有差异）：

- `long uptime`：系统从启动到现在的运行时间（单位：秒）。
- `unsigned long loads[3]`：分别表示 1 分钟、5 分钟、15 分钟内的系统平均负载（值为实际负载乘以 1000）。
- `unsigned long totalram`：系统总物理内存大小（单位：字节）。
- `unsigned long freeram`：空闲物理内存大小（单位：字节）。
- `unsigned long sharedram`：被共享使用的内存大小（单位：字节）。
- `unsigned long bufferram`：用于缓冲区的内存大小（单位：字节）。
- `unsigned long totalswap`：交换分区总大小（单位：字节）。
- `unsigned long freeswap`：空闲交换分区大小（单位：字节）。
- `unsigned short procs`：当前运行的进程数量。
- `unsigned long totalhigh`：高内存总大小（单位：字节）。
- `unsigned long freehigh`：空闲高内存大小（单位：字节）。
- `unsigned int mem_unit`：内存单位（字节），用于计算实际内存大小（部分系统可能不使用，直接以字节为单位）。

## 3.使用示例

以下是一个简单的示例，展示如何使用 `sysinfo` 获取并打印系统信息：

```c
#include <stdio.h>
#include <sys/sysinfo.h>

int main() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        perror("sysinfo failed");
        return 1;
    }

    printf("系统运行时间：%ld 秒\n", info.uptime);
    printf("1分钟平均负载：%lu\n", info.loads[0]);
    printf("5分钟平均负载：%lu\n", info.loads[1]);
    printf("15分钟平均负载：%lu\n", info.loads[2]);
    printf("总物理内存：%lu 字节\n", info.totalram);
    printf("空闲物理内存：%lu 字节\n", info.freeram);
    printf("当前进程数量：%hu\n", info.procs);

    return 0;
}
```

## 4.注意事项

1. **平台兼容性**：`sysinfo` 是 Linux 特有的函数，在 Windows 或其他 Unix 类系统（如` macOS`）中可能不支持或行为不同。
2. **权限要求**：获取系统信息通常不需要特殊权限，普通用户进程即可调用。
3. **信息精度**：`sysinfo` 返回的信息是调用时的快照，系统状态动态变化，因此结果仅反映当前时刻的情况。

如果需要更详细的系统信息（如进程详细信息、CPU 使用率等），可能需要结合其他函数（如 `getloadavg`、`proc` 文件系统读取等）使用。