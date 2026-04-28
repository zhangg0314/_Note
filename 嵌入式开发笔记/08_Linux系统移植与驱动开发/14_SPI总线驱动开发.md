# SPI 总线驱动开发

## 1. SPI 协议基础

SPI（Serial Peripheral Interface）是一种高速全双工同步串行通信总线。

### 1.1 四线模式

| 信号 | 全称 | 方向 | 说明 |
|------|------|------|------|
| SCLK | Serial Clock | Master→Slave | 时钟线 |
| MOSI | Master Out Slave In | Master→Slave | 主设备发送数据 |
| MISO | Master In Slave Out | Slave→Master | 从设备返回数据 |
| CS/SS | Chip Select | Master→Slave | 片选（低有效） |

### 1.2 四种工作模式（CPOL/CPHA）

| 模式 | CPOL（时钟极性） | CPHA（时钟相位） |
|------|-------------------|-------------------|
| 0 | 空闲低电平 | 第一个边沿采样 |
| 1 | 空闲低电平 | 第二个边沿采样 |
| 2 | 空闲高电平 | 第一个边沿采样 |
| 3 | 空闲高电平 | 第二个边沿采样 |

## 2. Linux SPI 子系统框架

与 I2C 子系统类似，分为三层：

- **SPI 控制器驱动层**：SOC 厂商提供，驱动芯片内部 SPI 控制器
- **SPI 核心层**：管理 `spi_master`、`spi_device`、`spi_driver`
- **SPI 设备驱动层**：驱动挂载在 SPI 总线上的二级外设

### 2.1 spi_master（控制器驱动）

```c
struct spi_master {
    struct device dev;
    u16 bus_num;
    u16 num_chipselect;
    u32 min_speed_hz;
    u32 max_speed_hz;
    int (*transfer)(struct spi_device *spi, struct spi_message *mesg);
};
```

### 2.2 spi_device（设备）

```c
struct spi_device {
    struct device dev;
    struct spi_master *master;
    u32 max_speed_hz;
    u8 chip_select;
    u8 mode;
    u8 bits_per_word;
    int irq;
};
```

设备树注册：

```dts
&spi1 {
    status = "okay";
    spidev@0 {
        compatible = "my,spi-device";
        reg = <0>;
        spi-max-frequency = <10000000>;
    };
};
```

### 2.3 spi_driver（驱动）

```c
struct spi_driver {
    const struct spi_device_id *id_table;
    int (*probe)(struct spi_device *spi);
    int (*remove)(struct spi_device *spi);
    struct device_driver driver;
};
```

## 3. SPI 数据传输接口

### 3.1 便捷读写函数

```c
#include <linux/spi/spi.h>

int spi_write(struct spi_device *spi, const void *buf, size_t len);
int spi_read(struct spi_device *spi, void *buf, size_t len);
int spi_write_then_read(struct spi_device *spi,
                         const void *txbuf, unsigned n_tx,
                         void *rxbuf, unsigned n_rx);
```

### 3.2 同步传输 API

```c
void spi_message_init(struct spi_message *msg);
void spi_message_add_tail(struct spi_transfer *t, struct spi_message *m);
int spi_sync(struct spi_device *spi, struct spi_message *message);
```

## 4. SPI 设备驱动开发步骤

1. 查阅原理图确定 SPI 通道号、片选、最大时钟频率、SPI 模式
2. 在设备树中添加从设备节点
3. 定义 `struct spi_driver`，实现 `probe`/`remove`
4. 注册驱动：使用 `module_spi_driver` 宏

```c
#include <linux/spi/spi.h>

static int my_spi_probe(struct spi_device *spi) { return 0; }
static int my_spi_remove(struct spi_device *spi) { return 0; }

static const struct of_device_id my_spi_of_match[] = {
    { .compatible = "my,spi-device" }, {}
};
MODULE_DEVICE_TABLE(of, my_spi_of_match);

static struct spi_driver my_spi_driver = {
    .probe  = my_spi_probe,
    .remove = my_spi_remove,
    .driver = {
        .name = "my_spi_driver",
        .of_match_table = my_spi_of_match,
    },
};
module_spi_driver(my_spi_driver);

MODULE_LICENSE("GPL");
```

## 5. 项目示例：W25Q64 SPI Flash 驱动

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define CMD_READ_ID     0x9F
#define CMD_READ_DATA   0x03
#define CMD_WRITE_EN    0x06
#define CMD_PAGE_PROG   0x02
#define CMD_READ_STATUS 0x05

struct w25q64_dev {
    struct spi_device *spi;
    struct cdev cdev;
    dev_t devno;
    struct mutex lock;
};

static int w25q64_read_regs(struct w25q64_dev *dev, u8 cmd,
                              void *buf, size_t len)
{
    struct spi_transfer t[2] = {
        { .tx_buf = &cmd, .len = 1 },
        { .rx_buf = buf, .len = len },
    };
    struct spi_message msg;
    spi_message_init(&msg);
    spi_message_add_tail(&t[0], &msg);
    spi_message_add_tail(&t[1], &msg);
    return spi_sync(dev->spi, &msg);
}

static int w25q64_read(struct w25q64_dev *dev, u32 addr,
                         void *buf, size_t len)
{
    u8 cmd[4] = { CMD_READ_DATA,
        (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF };
    struct spi_transfer t[2] = {
        { .tx_buf = cmd, .len = 4 },
        { .rx_buf = buf, .len = len },
    };
    struct spi_message msg;
    spi_message_init(&msg);
    spi_message_add_tail(&t[0], &msg);
    spi_message_add_tail(&t[1], &msg);
    return spi_sync(dev->spi, &msg);
}

static int w25q64_page_program(struct w25q64_dev *dev, u32 addr,
                                 const void *buf, size_t len)
{
    u8 cmd[4] = { CMD_PAGE_PROG,
        (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF };
    u8 status;
    int ret;

    ret = spi_write(dev->spi, &(u8){CMD_WRITE_EN}, 1);
    if (ret) return ret;

    struct spi_transfer t[2] = {
        { .tx_buf = cmd, .len = 4 },
        { .tx_buf = buf, .len = len },
    };
    struct spi_message msg;
    spi_message_init(&msg);
    spi_message_add_tail(&t[0], &msg);
    spi_message_add_tail(&t[1], &msg);
    ret = spi_sync(dev->spi, &msg);
    if (ret) return ret;

    do { w25q64_read_regs(dev, CMD_READ_STATUS, &status, 1); }
    while (status & 0x01);
    return 0;
}

static ssize_t w25q64_file_read(struct file *file, char __user *ubuf,
                                  size_t count, loff_t *ppos)
{
    struct w25q64_dev *dev = file->private_data;
    u8 *kbuf;
    int ret;

    if (*ppos >= 8 * 1024 * 1024) return 0;
    if (count > 4096) count = 4096;

    kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf) return -ENOMEM;

    mutex_lock(&dev->lock);
    ret = w25q64_read(dev, *ppos, kbuf, count);
    mutex_unlock(&dev->lock);

    if (ret == 0 && copy_to_user(ubuf, kbuf, count) == 0) {
        *ppos += count;
        ret = count;
    }
    kfree(kbuf);
    return ret;
}

static const struct file_operations w25q64_fops = {
    .owner = THIS_MODULE,
    .read  = w25q64_file_read,
};

static int w25q64_probe(struct spi_device *spi)
{
    struct w25q64_dev *dev;
    u8 id[3];
    int ret;

    dev = devm_kzalloc(&spi->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev) return -ENOMEM;

    dev->spi = spi;
    mutex_init(&dev->lock);
    spi_set_drvdata(spi, dev);

    ret = w25q64_read_regs(dev, CMD_READ_ID, id, 3);
    if (ret) return ret;
    dev_info(&spi->dev, "W25Q64 ID: %02x %02x %02x\n", id[0], id[1], id[2]);

    ret = alloc_chrdev_region(&dev->devno, 0, 1, "w25q64");
    if (ret) return ret;

    cdev_init(&dev->cdev, &w25q64_fops);
    dev->cdev.owner = THIS_MODULE;
    ret = cdev_add(&dev->cdev, dev->devno, 1);
    if (ret) unregister_chrdev_region(dev->devno, 1);
    return ret;
}

static int w25q64_remove(struct spi_device *spi)
{
    struct w25q64_dev *dev = spi_get_drvdata(spi);
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->devno, 1);
    return 0;
}

static const struct of_device_id w25q64_of_match[] = {
    { .compatible = "winbond,w25q64" }, {}
};
MODULE_DEVICE_TABLE(of, w25q64_of_match);

static struct spi_driver w25q64_driver = {
    .probe  = w25q64_probe,
    .remove = w25q64_remove,
    .driver = {
        .name = "w25q64",
        .of_match_table = w25q64_of_match,
    },
};
module_spi_driver(w25q64_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("W25Q64 SPI NOR Flash Driver");
```

**设备树节点**：

```dts
&spi1 {
    status = "okay";
    flash@0 {
        compatible = "winbond,w25q64";
        reg = <0>;
        spi-max-frequency = <50000000>;
    };
};
```

**测试**：

```bash
cat /proc/devices | grep w25q64
mknod /dev/w25q64 c <主设备号> 0
dd if=/dev/w25q64 of=flash_dump.bin bs=256 count=1
hexdump -C flash_dump.bin
```
