# IO设备模型

[I/O设备模型](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/programming-manual/device/device)

# PIN设备

## 1.RT-Thread API

|      **函数**       |                  **描述**                   |
| :-----------------: | :-----------------------------------------: |
|    **文件位置**     | **rt-thread\components\drivers\misc\pin.c** |
|  rt_hw_pin_init()   |                 初始化引脚                  |
|    rt_pin_get()     |                获取引脚编号                 |
|    rt_pin_mode()    |                设置引脚模式                 |
|   rt_pin_write()    |                设置引脚电平                 |
|    rt_pin_read()    |                读取引脚电平                 |
| rt_pin_attach_irq() |            绑定引脚中断回调函数             |
| rt_pin_irq_enable() |                使能引脚中断                 |
| rt_pin_detach_irq() |            脱离引脚中断回调函数             |

## 2.调用关系

```c
rt_hw_pin_init-->
    rt_device_pin_register-->（设备管理层）
    	_hw_pin.ops = _at32_pin_ops-->
    		at32_pin_xx-->
    			gpio_bits_write(芯片标准库)
    	rt_device_register(设备框架层)
========================================================================
rt_pin_xx -->（设备管理层）
	static struct rt_device_pin _hw_pin;
	_hw_pin.ops->pin_mode;
	_hw_pin.ops->pin_mode;
	_hw_pin.ops->pin_write;
	_hw_pin.ops->pin_read;
	_hw_pin.ops->pin_attach_irq;
	_hw_pin.ops->pin_detach_irq;
	_hw_pin.ops->pin_irq_enable;
	_hw_pin.ops->pin_get;
```

| **函数**                    | **描述**             |
| --------------------------- | -------------------- |
| rt_device_find()            | 查找设备             |
| rt_device_open()            | 打开设备             |
| rt_device_read()            | 读取数据             |
| rt_device_write()           | 写入数据             |
| rt_device_control()         | 控制设备             |
| rt_device_set_rx_indicate() | 设置接收回调函数     |
| rt_device_set_tx_complete() | 设置发送完成回调函数 |
| rt_device_close()           | 关闭设备             |

# UART设备

## 1.RT-Thread API

|          **函数**           |       **描述**       |
| :-------------------------: | :------------------: |
|      rt_device_find()       |       查找设备       |
|      rt_device_open()       |       打开设备       |
|      rt_device_read()       |       读取数据       |
|      rt_device_write()      |       写入数据       |
|     rt_device_control()     |       控制设备       |
| rt_device_set_rx_indicate() |   设置接收回调函数   |
| rt_device_set_tx_complete() | 设置发送完成回调函数 |
|      rt_device_close()      |       关闭设备       |

## 2.调用关系

### 1.初始化

```c

```

