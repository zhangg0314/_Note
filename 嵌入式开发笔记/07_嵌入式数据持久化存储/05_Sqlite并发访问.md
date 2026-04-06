# 文件锁-进程间互斥

```c++
inline bool unlock_file(int fdesc)
{
   return (0 == flock(fdesc, LOCK_UN));
}

inline bool readlock_file(int fdesc)
{
   return (0 == flock(fdesc, LOCK_SH));
}

inline bool writelock_file(int fdesc)
{
   return (0 == flock(fdesc, LOCK_EX));
}
```

## 函数作用

表格





| 函数               | 功能         | 锁类型                                 |
| :----------------- | :----------- | :------------------------------------- |
| `unlock_file()`    | 释放文件锁   | `LOCK_UN` - 解锁                       |
| `readlock_file()`  | 获取共享读锁 | `LOCK_SH` - 共享锁（多个进程可同时读） |
| `writelock_file()` | 获取独占写锁 | `LOCK_EX` - 排他锁（独占访问）         |

## 关键特性

### 1. **flock 的行为特点**

- **建议性锁**：依赖进程配合，不强制阻止 I/O
- **进程级**：同一进程内多次加锁不会阻塞自己
- **自动释放**：进程退出时自动解锁

### 2. **使用模式**

cpp

复制

```cpp
int fd = open("data.txt", O_RDWR);

// 读操作
if (readlock_file(fd)) {
    // 安全读取数据...
    read(fd, buffer, size);
    unlock_file(fd);  // 必须解锁
}

// 写操作
if (writelock_file(fd)) {
    // 独占写入数据...
    write(fd, data, size);
    unlock_file(fd);  // 必须解锁
}
```

## ⚠️ 重要注意事项

### 问题 1：阻塞行为（默认阻塞）

当前实现是**阻塞的**：

- 如果文件已被锁定，`flock()` 会阻塞等待
- 需要非阻塞版本时加 `LOCK_NB` 标志

### 问题 2：缺少错误处理

建议增强版本：

cpp

复制

```cpp
#include <errno.h>

// 非阻塞版本
inline bool try_readlock_file(int fdesc)
{
    return (0 == flock(fdesc, LOCK_SH | LOCK_NB));
}

inline bool try_writelock_file(int fdesc)
{
    return (0 == flock(fdesc, LOCK_EX | LOCK_NB));
}

// 带超时的版本（伪代码）
bool timed_writelock_file(int fdesc, int timeout_sec)
{
    // 轮询或信号量实现超时逻辑
}
```

### 问题 3：异常安全

**推荐使用 RAII 封装**：

cpp

复制

```cpp
class FileLockGuard {
    int fd_;
    bool locked_;
public:
    explicit FileLockGuard(int fd, bool write = false) 
        : fd_(fd), locked_(false) {
        locked_ = write ? writelock_file(fd) : readlock_file(fd);
    }
    
    ~FileLockGuard() {
        if (locked_) unlock_file(fd_);
    }
    
    bool isLocked() const { return locked_; }
    
    // 禁止拷贝
    FileLockGuard(const FileLockGuard&) = delete;
    FileLockGuard& operator=(const FileLockGuard&) = delete;
};

// 使用示例
{
    FileLockGuard lock(fd, true);  // 写锁
    if (lock.isLocked()) {
        // 安全操作，自动解锁
    }
} // 自动解锁
```

## 适用场景

表格





| 适合                 | 不适合                             |
| :------------------- | :--------------------------------- |
| 单服务器多进程协调   | 分布式多机环境                     |
| 简单的读写互斥       | 需要强制 I/O 阻塞                  |
| 同一系统内的进程同步 | 线程级细粒度锁（`flock` 是进程级） |