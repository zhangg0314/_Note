<<<<<<< HEAD
# 698.42规约

##  1.术语与定义

- 主节点：集中器所在得本地通信模块
- 从节点：采集器或电能表所在的本地通信模块
- 从节点附属节点：指与从节点具有绑定关系的附加设备，简称附属节点

## x.无数据单元组帧（下行报文）

### 1.组帧函数

```c++
static int FrmBase(uint8_t *poutbuf,int buflen,int afn,int dt,uint8_t *pusrbuf,int usrlen);
//不带数据单元的，都是集中器给集中器模块的，所以操作的是主节点
```

### 2.上层函数

- **初始化（AFN=01H）**

  ```c++
  static int FrmInitDev(uint8_t *pbuf,int buflen,int inittype);//设备初始化
  ```

- da'wdaw

- **查询数据（AFN=03H）**

  ```c++
  // 查询数据（AFN=03H）
  enum {
      IQR_DEV_INFO			= 0x01, //厂商代码和版本信息
      IQR_NOISE				= 0x02,//噪声值
      IQR_SNODE_INFO			= 0x03,//从节点侦听信息
      IQR_MNODE_ADDR			= 0x04,//主节点地址
      IQR_MNODE_STAT			= 0x05,//主节点状态字和通信速率
      IQR_MNODE_DISTURB		= 0x06,//主节点干扰状态
      IQR_MAXRANDTIME          = 0x07,//读取从节点监控最大超时时间
      IQR_WIFIPARAM            = 0x08,//查询无线通讯参数
      
      IQR_DELAYTIME            = 0x09,//通信演示相关的广播时长查询
      IQR_WORKMODE_INFO        = 0x0A,//本地通讯模块运行模式信息
      IQR_MSGSUPPORT_INFO      = 0x0B,//本地通信木块AFN索引
      IQR_LOCAL_HPLC_MID_INFO  = 0xC,//12本地HPLC mid 信息
      IQR_BDFREQ_INFO          = 0x10,//宽带载波频段序号
      IQR_PAIR_PARAM           = 0x15,
      IQR_NODE_AREA_INFO       = 0x65,//使用搜表后的数量查询
      IQR_OFFLINE_DETECT       = 201, //F201查询从节点状态变化上报使能标志(山西扩展)查询STA认证使能开关（AFN=03H FN=201）
      IQR_STA_PROFILE          = 102 //查询CCO模块曲线数据（AFN=03H Fn=102，新增）
  };
  ```

- 











## x.核心组帧函数

### 1.用户数据区定义（下行报文）

- 路由数据转发类（`AFN=0x13`）
- `fn = 1`监控从节点` fn = 2~248`备用
- 数据单元格式
  ![image-20250705202315466](..\figure\image-20250705202315466.png)
  ![image-20250705202423100](..\figure\image-20250705202423100.png)

### 2.组帧函数

```c++
int FrmStruct::FrmRouterRand(uint8_t *pbuf, int buflen, uint8_t *psrcaddr, uint8_t *pdstaddr, appreqrand_t &appreqrand,bool bSupport698)
{
    usrinfo_dn_t stmp;									//信息域R
    memset(&stmp, 0, sizeof(stmp));
    if (vender == RISECOMM42) //工作在路由模式
        stmp.rtemode			=	WO_ROUTER;
    else
        stmp.rtemode			=	W_ROUTER;
    stmp.subnode = WO_SUBNODE; //无附属从节点
    stmp.module	 =	MTR_MODULE;//从节点模块
    stmp.detec  = WO_CD; //不带冲突检测
    stmp.relay  = 0; //无中继路由
    stmp.chann  = 0; //不区分信道
    stmp.dbgcode  = NO_CODE; //无纠错编码
    stmp.rtnchars  = 0x28; //预计应答字节数
    stmp.speed  = 0x32; //默认速率
    stmp.unit = BPS;
    stmp.frmseq = alloc_frame_index();

    frmctrl_t sctl;								     //控制域C
    memset(&sctl, 0, sizeof(sctl));
    if (MTR_MODE_PARALLEL == g_mtr_mode)
    {
        sctl.comway = BROADBAND;
    }
    else
    {
        sctl.comway = CENTRALIZED; //窄带PLC主从式
    }
    sctl.dir = DNLNK; //集中器下行
    sctl.prm = INITIATIVE; //启动站
    
    if( D42_698_45 == appreqrand.protocol )			//appreqrand == 数据单元内容
    {
        if(!bSupport698)
        {
            appreqrand.protocol	=	D42_BYPASS;
        }
    }
    return FrmCommon(pbuf, buflen, stmp, sctl, psrcaddr, pdstaddr, ROUTER_FWD, FWD_MON_SNODE, (uint8_t *)&appreqrand, CalUsrdatalen(appreqrand));
}
```

=======
# 下行抄表

## 1.`APP`介绍

`gatherexe`是用于执行下行抄表的`APP`。它主要是通过消息队列来接收其他进程发送的抄表消息来解析，并通过与本地载波模块串口或485串口来与电表进行交互。

1. `gatherexe`进程中创建了一个`dlt698_42`线程，他们之间通过`udpskt`（`UDP`套接字）来进行通信。主进程赋值监听消息队列或者检查抄表任务配置数据库来看是否有抄表任务需要执行，然后做进一步处理，通过`UDP`套接字发给线程，由线程来执行。
2. `dlt698_42`线程通过`select`函数监听来自上行的`gathexe`进程的消息和来自下行的串口消息。下行串口可以是485串口也可以是载波模块的串口。

## 2.抄表通道

由于通道自身的特性，485仅随抄和定抄，而载波可以进行广播、并抄、随抄和定抄。

### 1.485



### 2.载波
