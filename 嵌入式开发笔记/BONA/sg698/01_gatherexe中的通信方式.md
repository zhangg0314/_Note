# 通信模块的通信方式

通信方式是指集中器下行的通信模块所采用的通信方式类型，不同的通信方式决定用户数据区中的

数据构成和格式，主要有下面这些通信方式：

```c++
typedef enum {
    RESERVED			=	0x00,//保留
    CENTRALIZED			=	0x01,//集中式路由载波通信
    DISTRIBUTED			=	0x02,//分布式路由载波通信
    BROADBAND			=	0x03,//HPLC 载波通信
    ESROUTER            =   0x07,//窄带PLC主从式(东软载波)
    WIRELESS			=	0x0a,//微功率无线通信
    ETHERNET			=	0x14//以太网通信
} com_t;
```

# 进程、线程、载波模块间的通信

## 1.文件位置

```shell
#(软路由通信)
/sg698/gathermeter/unify42/udpskt.h
/sg698/gathermeter/unify42/udpskt.cpp

#（串口通信）
/sg698/gathermeter/unify42/amrserial.h
/sg698/gathermeter/unify42/amrserial.cpp
```

## 2.通信方式介绍

```c++
typedef enum {
	UDPSKT_DNLNK = 0,//线程到本地模块(宽带载波不是用UDP)
	UDPSKT_UPLNK, //线程接收进程，并发到进程（线程调用）
	UDPSKT_EXTLNK, //进程接收线程，并发到线程（进程调用）
	UDPSKT_NBR,
} udpskt_type_t;
```

- `gatherexe`进程到`dlt698_42`线程

  ```c++
  UDPSKT_EXTLNK, //进程发到线程（UPD套接字）
  ```

- `dlt698_42`线程到`gatherexe`进程

  ```c++
  UDPSKT_UPLNK, //线程发到进程（UPD套接字）
  ```

- `dlt698_42`线程到本地模块

  ```c++
  if(模块类型 ==  软路由)
      UDPSKT_DNLNK = 0,//线程到本地模块(宽带载波不是用UDP)
  else if(模块类型 == 载波)
      串口通信
  
      //！！！模块类型是软路由还是载波，由集中器环境变量PLCTYPE来设置
  modtype     = CARRIer;
  es_type			=	ES_STANDARD;
  char *type = getenv("PLCTYPE");
  if (type != NULL)
  {
      if (strcmp(type, "unify42") == 0)
      {
          modtype     = CARRIer;
          es_type			=	ES_STANDARD;
      }
      else if (strcmp(type, "eastsoftCOMP") == 0) // 为东软三代兼容模式
      {
          modtype     = CARRIer;
          es_type			=	ES_COMPATIBLE;
      }
      else if (strcmp(type, "eastsoftExt") == 0) // 为东软过零检测模式
      {
          modtype     = CARRIer;
          es_type			=	ES_ZEROTEST;
      }
      else
      {
          modtype 		=	SOFTROUTER;
      }
  }
  ```


# 进程——>线程
