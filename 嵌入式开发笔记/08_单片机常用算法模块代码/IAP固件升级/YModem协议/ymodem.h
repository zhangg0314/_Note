/**
  **************************************************************************
  * @file     ymodem.h
  * @brief    ymodem header file
  **************************************************************************
  */

#ifndef _YMODEM_H_
#define _YMODEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f403a_407_board.h"
#include "flash.h"

/** @addtogroup USART_iap_bootloader
  * @{
  */
/** @defgroup bootloader_definition
  * @{
  */
#define PACKET_SEQNO_INDEX      (1)//packet number,包号索引值
#define PACKET_SEQNO_COMP_INDEX (2)//packet number’complement，包号反码索引值 

#define PACKET_HEADER_LEN       (3)
#define PACKET_TRAILER_LEN      (2)

#define PACKET_OVERHEAD_LEN     (PACKET_HEADER_LEN + PACKET_TRAILER_LEN)

#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)

#define FILE_NAME_LENGTH        (256) //256个Byte的文件名长度
#define FILE_SIZE_LENGTH        (16)  //16个Byte的文件长度(每个Byte代表十进制中的1位)

#define SOH                     (0x01)  //128字节数据包开始
#define STX                     (0x02)  //1024字节的数据包开始
#define EOT                     (0x04)  //结束传输
#define ACK                     (0x06)  //回应
#define NAK                     (0x15)  //没回应
#define CAN                     (0x18)  //取消传输命令。连续发送5个该命令
#define C_CHAR                  (0x43)  //'C' == 0x43, 请求开始或结束时需要 16-bit CRC 


#define NAK_TIMEOUT             (0x5000)//5秒超时时间
#define MAX_ERRORS              (3)//最大重试次数
 /**
  * @}
  */
 /** @defgroup bootloader_typedefinition
  * @{
  */
 /**
 * @brief Ymodem返回值枚举（标准化错误码）
 */
 typedef enum
 {
   YMODEM_OK = 0,              // 操作成功
   YMODEM_ERR_TIMEOUT = -1,    // 超时错误
   YMODEM_ERR_FRAME = -2,      // 帧格式错误
   YMODEM_ERR_FILE_SIZE = -3,  // 文件大小错误
   YMODEM_ERR_STORAGE = -4,    // 存储操作错误
   YMODEM_ERR_CANCELLED = -5,  // 传输被取消
   YMODEM_ERR_UNKNOWN = -6     // 未知错误
 } 
 Ymodem_Status;
 /**
  * @brief 硬件接口抽象结构体（需要根据实际硬件实现）
  */
 typedef struct
 {
   void (*send_byte)(uint16_t data);                       // 发送一个字节
   void (*send_buff)(void *buff, uint64_t len);            // 发送一个串
   uint8_t (*recv_byte)(uint8_t *data, uint64_t len, uint32_t timeout); // 接收一个字节（timeout单位：毫秒），返回值：1-成功接收，0-超时
   // 存储写入函数（addr：地址，data：），返回值：YMODEM_OK-成功，其他-失败
   void (*storage_write)(uint32_t addr, uint8_t *data);
   void (*delay_ms)(uint16_t ms); // 延时函数（单位：毫秒）
 } 
 Ymodem_Hal_t;
 /**
  * @brief Ymodem管理结构体
  */
typedef struct
{
   Ymodem_Hal_t* hal;
   uint8_t m_file_name[FILE_NAME_LENGTH]; // 存储文件名的字符数组
   uint8_t m_end_flag;                    // 结束标志
   uint8_t *m_file_buf;                   // 数据缓冲区（外部分配）
   uint32_t m_file_buf_size;              // 缓冲区大小
  
   uint32_t m_tmp_len;                    // 缓冲区当前数据长度
   uint32_t received_size;                // 已接收大小
   uint32_t m_file_size;                  // 文件总大小
} 
Ymodem_Handle_t;
  /**
  * @}
  */
 /**
  * @}
  */
 /** @defgroup bootloader_exported_functions
  * @{
  */
extern Ymodem_Status Ymodem_Init(Ymodem_Handle_t *handle, Ymodem_Hal_t *hal, uint8_t *buf, uint32_t buf_size);
extern Ymodem_Status Ymodem_Receive(Ymodem_Handle_t *handle,uint32_t write_addr);
 /**
  * @}
  */

 /**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif
