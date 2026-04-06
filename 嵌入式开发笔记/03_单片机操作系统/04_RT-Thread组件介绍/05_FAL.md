

# FAL组件介绍

FAL（Flash Abstraction Layer）是RT-Thread实时操作系统中用于统一管理Flash存储设备的中间件组件。它通过抽象层设计屏蔽了不同Flash芯片的底层差异，为开发者提供标准化的操作接口。并具有以下特性：

- 支持静态可配置的分区表，并可关联多个 Flash 设备；

  ```
  一个设备可以挂载多个flash设备，包括片内烧写程序的flash，
  然后一个分区又能映射到一个flash设备上
  ```

- 分区表支持自动装载。
  避免在多固件项目，分区表被多次定义的问题；

  ```
  “多固件项目” 指一个嵌入式设备中包含多个独立固件（如：Bootloader 固件、App 应用固件、OTA 升级固件、备份固件等），或一个项目下编译多个不同功能的固件（如：量产版固件、调试版固件）。
  
  “自动装载” 指：将分区表从每个固件的代码中独立出来，作为一个 “全局统一的配置源”（如单独的配置文件、固化在 Flash 指定地址的二进制表、或编译期的统一宏定义），所有固件运行时自动从这个统一源加载分区表，而非在各自代码中重复定义。
  ```

- 代码精简，对操作系统无依赖 ，可运行于裸机平台，比如对资源有一定要求的 Bootloader；

- 统一的操作接口。保证了文件系统、OTA、NVM（例如：EasyFlash） 等对 Flash 有一定依赖的组件，底层 Flash - 驱动的可重用性；

- 自带基于 Finsh/MSH 的测试命令，可以通过 Shell 按字节寻址的方式操作（读写擦） Flash 或分区，方便开发者进行调试、测试；
  ![img](..\figure\145b0fa0e5474d7c97290fa8015b9072.png)

# 核心功能架构

![img](..\figure\b1ba094f66764264bd04ea2eb7a1dad1.png)

## 1.设备抽象层

将不同厂商的Flash设备（如NorFlash、NandFlash）统一抽象为三类操作：

```c
//读操作
    fal_partition_read()
//写操作
    fal_partition_write()
//擦除操作
    fal_partition_erase()
```

## 2.分区管理机制

通过分区表定义存储空间逻辑划分：

```c
#define NOR_FLASH_DEV_NAME             "norflash0"
#define ONCHIP_FLASH_DEV_NAME 		   "onchipflash0"

/* ====================== Partition Configuration ========================== */
/* partition table */
// 静态分区表定义
const struct fal_partition fal_partitions[] = {
    { "app",        NOR_FLASH_DEV_NAME,  0x00000,  0x100000 }, // 应用固件分区（1M）
    { "param",      NOR_FLASH_DEV_NAME,  0x100000, 0x20000  }, // 参数分区（128K）
    { "log",        ONCHIP_FLASH_DEV_NAME,0x00000, 0x10000 }, // 日志分区（64K）
};
```

每个分区包含：**起始地址**、**大小**、**所属物理设备等元数据**。

## 3.统一API接口

### 1.查找 Flash 设备

```c
const struct fal_flash_dev *fal_flash_device_find(const char *name)
```

### 2.查找 Flash 分区

```c
const struct fal_partition *fal_partition_find(const char *name)
```

### 3.获取分区表

```c
const struct fal_partition *fal_get_partition_table(size_t *len)
```

### 4.临时设置分区表

```c
void fal_set_partition_table_temp(struct fal_partition *table, size_t len)
```

### 5.从分区读取数据

```c
int fal_partition_read(const struct fal_partition *part, uint32_t addr, uint8_t *buf, size_t size)
```

### 6.往分区写入数据

```c
int fal_partition_write(const struct fal_partition *part, uint32_t addr, const uint8_t *buf, size_t size)
```

### 7.擦除分区数据

```c
int fal_partition_erase(const struct fal_partition *part, uint32_t addr, size_t size)
```

### 8.打印分区表

```c
void fal_show_part_table(void)
```

### 9.创建块设备

```c
struct rt_device *fal_blk_device_create(const char *parition_name)
```

### 10.创建 MTD Nor Flash 设备

```c
struct rt_device *fal_mtd_nor_device_create(const char *parition_name)
```

### 11.创建字符设备

```c
struct rt_device *fal_char_device_create(const char *parition_name)
```

# 关键组件结构

![image-20251215163057656](..\figure\image-20251215163057656.png)

- 设备驱动层：实现struct fal_flash_dev操作集
- 中间抽象层：处理块对齐、地址映射等逻辑
- 应用接口层：提供分区级别的读写控制

# 典型应用场景

## 1.OTA固件升级

通过分区切换实现双备份升级：“双备份升级”（也叫 A/B 分区升级、双 App 分区升级）是嵌入式 OTA 的主流安全方案：

- 设备 Flash 中划分两个独立的应用分区：`app`（当前运行的主分区）和`app_backup`（备份分区，也常命名为`ota`）；

```c
fal_partition_t app = fal_partition_find("app");
fal_partition_erase(app, 0, app->len);
fal_partition_write(app, 0, new_firmware, firmware_size);
```

## 2.参数存储系统

结合EasyFlash等组件实现键值对存储：

```c
ef_set_env("wifi_ssid", "RT-Thread_AP");
```

## 3.文件系统挂载

为LittleFS/DFS提供块设备支持：

```c
struct rt_device *flash_dev = fal_mtd_device_create("filesys");
```

# 关键注意事项

## 1.对齐限制

写操作需满足最小写入单位要求（通常为256字节），数学表达：

```c
addr mod δ = 0 ( δ= min_write_size) //即地址是最小写入单位的整数倍
```

## 2.擦除预处理

写操作前必须擦除目标扇区，擦除粒度满足：

```c
size mod ϵ=0 (ϵ=sector_size)
```

## 3.线程安全性

多线程操作需加锁：

```c
rt_mutex_take(&flash_mutex, RT_WAITING_FOREVER);
fal_partition_write(...);
rt_mutex_release(&flash_mutex);
```

# ——实践操作——

# FAL移植

## 1.组件配置

![image-20251216111232387](..\figure\image-20251216111232387.png)

每个功能的配置说明如下：

- 开启调试日志输出（**默认开启**）

- 分区表是否在 fal_cfg.h 中定义（**默认开启**）。
  如果关闭此选项，fal 将会自动去指定 Flash 的指定位置去检索并装载分区表，具体配置详见下面两个选项![image-20251216111628491](..\figure\image-20251216111628491.png)

  ```
  1.存放分区表的 Flash 设备；
  
  2.分区表的结束地址 即位于 Flash 设备上的偏移。fal 将从此地址开始往回进行检索分区表，直接读取到 Flash 顶部。如果不确定分区表具体位置，这里也可以配置为 Flash 的结束地址，fal 将会检索整个 Flash，检索时间可能会增加。
  ```

- 启用 FAL 针对 SFUD 的移植文件（**默认关闭**）；

  1）应输入调用 rt_sfud_flash_probe 函数时传入的 FLASH 设备名称（也可以通过 list_device 命令查看 Block Device 的名字获取）。
  2）**该名称与分区表中的 Flash 名称对应**，只有正确设置设备名字，才能完成对 FLASH 的读写操作。
  3）图形化配置只支持配置一个flash设备，如果需要配置多个flash，则需在配置文件中定义。

- 然后让 RT-Thread 的包管理器自动更新，或者使用 pkgs --update 命令更新包到 BSP 中。

## 2.硬件初始化

先初始化spi，加载spi的驱动，将flash设置挂载到spi总线上，然后SPI再通过SUFD的接口初始化flash设备，加载操作flash设置的驱动。

```c
rt_err_t spim_init(void)
{
    rt_err_t result;
    result = rt_hw_spi_device_attach("spi1", "spi10", GPIOA, GPIO_PINS_4);
    if (result != RT_EOK)
    {
        LOG_E("attach sfud_flash failed!...\n");
        return result;
    }
    // spi10 表示挂载在 spi1 总线上的 0 号设备
    /* 使用 SFUD 探测 spi10 从设备，并将 spi10 连接的 flash 初始化为块设备，名称 W25Q128 */
    if (RT_NULL == (rtt_dev = rt_sfud_flash_probe("W25Q64", "spi10")))
    {
        LOG_E("probe sfud_flash failed!...\n");
        return -RT_ERROR;
    };
    return RT_EOK;
}
```

## 3.注册Flash设备

在定义 Flash 设备表前，需要先定义 Flash 设备。可以是片内 flash, 也可以是片外基于 SFUD 的 spi flash：

![image-20251216112735738](..\figure\image-20251216112735738.png)

定义具体的 Flash 设备对象，用户需要根据自己的 Flash 情况分别实现 init、 read、 write、 erase 这些操作函数。

如果配置seting的时候选择了使用SUFD，则实现这些操作函数就使用SUFD的接口，**如果未使用SUFD，需要先实现flash的具体的读写操作函数**。

```c
struct fal_flash_dev w25q64 =
{
    .name       = FAL_USING_NOR_FLASH_DEV_NAME,
    .addr       = 0,
    .len        = 8 * 1024 * 1024,
    .blk_size   = 4096,
    .ops        = {init, read, write, erase},
    .write_gran = 1
};
```

```c
static int init(void)
{

#ifdef RT_USING_SFUD
    /* RT-Thread RTOS platform */
    sfud_dev = rt_sfud_flash_find_by_dev_name(FAL_USING_NOR_FLASH_DEV_NAME);
#else
    /* bare metal platform */
    extern sfud_flash sfud_norflash0;
    sfud_dev = &sfud_norflash0;
#endif

    if (NULL == sfud_dev)
    {
        return -1;
    }

    /* update the flash chip information */
    w25q64.blk_size = sfud_dev->chip.erase_gran;
    w25q64.len = sfud_dev->chip.capacity;

    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    sfud_read(sfud_dev, w25q64.addr + offset, size, buf);

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    if (sfud_write(sfud_dev, w25q64.addr + offset, size, buf) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

static int erase(long offset, size_t size)
{
    assert(sfud_dev);
    assert(sfud_dev->init_ok);
    if (sfud_erase(sfud_dev, w25q64.addr + offset, size) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}
```

## 3.创建分区表

![image-20251216113020736](..\figure\image-20251216113020736.png)

```c
FAL_PART_TABLE_DEFINE(partition_table)
```

## 4.初始化FAL

![image-20251216113317530](..\figure\image-20251216113317530.png)

![image-20251216113142868](..\figure\image-20251216113142868.png)

## 5.执行存储操作

```c
fal_partition_t param = fal_partition_find("params");
uint8_t buffer[128];
fal_partition_read(param, 0, buffer, sizeof(buffer));
```

# ——参考链接——

[rt-thread FAL组件详解_rtthread fal-CSDN博客](https://blog.csdn.net/qq_15181569/article/details/149714516)