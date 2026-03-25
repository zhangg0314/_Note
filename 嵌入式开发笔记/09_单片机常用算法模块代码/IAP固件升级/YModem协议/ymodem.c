#include "ymodem.h"
#include "common.h"
#include "flash.h"
#include "WDT.h"
static uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD_LEN];
/**
 * @brief  发送取消Ymodem传输命令，连续发两个CAN信号
 * @param  none
 * @retval none
 */
static void Ymodem_Cancel(Ymodem_Handle_t *handle)
{
    handle->hal->send_byte(CAN);
    handle->hal->send_byte(CAN);
}
/**
 * @brief  发送确认信号，发送ACK信号
 * @param  none
 * @retval none
 */
static void Ymodem_Acknowledge(Ymodem_Handle_t *handle)
{
    handle->hal->send_byte(ACK);
}
/**
 * @brief  发送否认信号，发送NAK信号
 * @param  none
 * @retval none
 */
static void Ymodem_NegativeAcknowledge(Ymodem_Handle_t *handle)
{
    handle->hal->send_byte(NAK);
}
/**
 * @brief  发送请求信号，发送字符C
 * @param  none
 * @retval none
 */
static void Ymodem_Request(Ymodem_Handle_t *handle)
{
    handle->hal->send_byte(C_CHAR);
}
/**
 * @brief  Ymodem协议专用CRC16校验（多项式0x1021）
 * @param  data: 待校验数据缓冲区
 * @param  length: 数据长度（字节数）
 * @retval 16位CRC校验值
 */
static uint16_t Ymodem_CRC16(const uint8_t *data, uint32_t length)
{
    uint8_t i;
    uint16_t crc = 0x0000; // 初始值固定为0x0000
    while (length--)
    {
        // 高8位与当前数据字节异或
        crc ^= (uint16_t)(*data++) << 8;
        // 逐位处理
        for (i = 0; i < 8; i++)
        {
            // 若最高位为1，左移后异或多项式0x1021
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                // 否则仅左移
                crc <<= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief   从发送端接收一个完整数据包
 * @param  handle: Ymodem管理结构体
 *         data ：数据指针
 *         length：长度
 *         timeout ：超时时间
 *         packets_received:已接收的包数
 * @retval Ymodem_Status
 */
static Ymodem_Status Receive_Packet(Ymodem_Handle_t *handle, uint8_t *data, int32_t *length, uint32_t timeout, int32_t packets_received)
{
    uint8_t _start_ch; // 接收帧的第一个字节或者控制信号
    uint16_t packet_size, crc;
    if (handle->hal->recv_byte(&_start_ch, 1, timeout) != TRUE) // 接收完整包的第一个字节并判断
    {
        return YMODEM_ERR_TIMEOUT;
    }
    *length = 0;
    switch (_start_ch)
    {
    // 根据帧头判断帧类型
    case SOH:
        packet_size = PACKET_SIZE;
        break;
    case STX:
        packet_size = PACKET_1K_SIZE;
        break;
    case EOT:
        *length = 0;
        return YMODEM_OK;
    case CAN:
        if ((handle->hal->recv_byte(&_start_ch, 1, timeout) == TRUE) && (_start_ch == CAN))
        {
            *length = -1;
            return YMODEM_OK;
        }
        else if (_start_ch != CAN)
        {
            return YMODEM_ERR_UNKNOWN;
        }
        else
        {
            return YMODEM_ERR_TIMEOUT;
        }
    default:
        return YMODEM_ERR_UNKNOWN;
    }
    data[0] = _start_ch;

    if (handle->hal->recv_byte(data + 1, packet_size + PACKET_OVERHEAD_LEN - 1, timeout) != TRUE)
    {
        return YMODEM_ERR_TIMEOUT;
    }
    // packets_received期望接收的下一包的序号
    if (handle->m_end_flag == 1)
    {
        packets_received = 0;
    }
    if ((*(data + PACKET_SEQNO_INDEX) & 0xff) != (packets_received & 0xff))
    {
        return YMODEM_ERR_FRAME;
    }
    if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)) // 包号出错判断
    {
        return YMODEM_ERR_FRAME;
    }

    crc = Ymodem_CRC16(&data[PACKET_HEADER_LEN], packet_size);
    if (crc != (uint16_t)((data[packet_size + PACKET_OVERHEAD_LEN - 2] << 8) | data[packet_size + PACKET_OVERHEAD_LEN - 1]))
    {
        return YMODEM_ERR_FRAME;
    }
    *length = packet_size;
    return YMODEM_OK;
}
/**
 * @brief   解析一个数据包(起始帧或者数据帧)
 * @param  handle: Ymodem管理结构体
 *         packet_data ：数据包首地址指针
 *         packet_data_length：数据包的数据域长度
 *         packets_received:已经接收的包数量
 * @retval Ymodem_Status
 */
static Ymodem_Status Packet_Parser(Ymodem_Handle_t *handle, uint8_t *packet_data, int32_t packet_data_length, int32_t packets_received)
{
    uint8_t *file_ptr;
    uint8_t file_size[FILE_SIZE_LENGTH];
    int32_t i;

    if (packets_received == 0) // 文件名数据包
    {
        // 文件名数据包有效数据区域
        for (i = 0, file_ptr = packet_data + PACKET_HEADER_LEN; (*file_ptr != 0) && (i < FILE_NAME_LENGTH);)
        {
            handle->m_file_name[i++] = *file_ptr++;
        }
        handle->m_file_name[i++] = '\0';

        for (i = 0, file_ptr++; (*file_ptr >= '0' && *file_ptr <= '9') && (i < FILE_SIZE_LENGTH) && (*file_ptr != 0);)
        {
            file_size[i++] = *file_ptr++;
        }
        file_size[i++] = '\0';
        if (Str2UInt(file_size, &handle->m_file_size) != 0)
        {
            return YMODEM_ERR_FILE_SIZE;
        }
        // 测试数据包是否过大
        if (handle->m_file_size > (FLASH_SIZE * 1024 - 1))
        {
            return YMODEM_ERR_FILE_SIZE;
        }
        return YMODEM_OK;
    }
    // 完整一帧数据包
    else
    {
        if (handle->m_end_flag == 0)
        {
            memcpy(handle->m_file_buf + handle->m_tmp_len, packet_data + PACKET_HEADER_LEN, packet_data_length);
            handle->m_tmp_len += packet_data_length;
            handle->received_size += packet_data_length;
        }
        return YMODEM_OK;
    }
}
/**
 * 初始化Ymodem
 */
Ymodem_Status Ymodem_Init(Ymodem_Handle_t *handle, Ymodem_Hal_t *hal, uint8_t *buf, uint32_t buf_size)
{
    if (!handle || !hal || !buf || buf_size == 0)
    {
        return YMODEM_ERR_UNKNOWN;
    }

    if (!hal->send_byte || !hal->recv_byte || !hal->storage_write || !hal->delay_ms)
    {
        return YMODEM_ERR_UNKNOWN;
    }

    memset(handle, 0, sizeof(Ymodem_Handle_t));
    handle->hal = hal;
    handle->m_file_buf = buf;
    handle->m_file_buf_size = buf_size;
    return YMODEM_OK;
}
/**
 * @brief  通过 ymodem协议接收一个完整文件数据
 * @param  buf: 首地址指针
 * @retval  size 文件长度
 *          0    发送端取消
 *          -1   接收端自身原因取消
 *          -2   接收超时
 */
Ymodem_Status Ymodem_Receive(Ymodem_Handle_t *handle, uint32_t write_addr)
{
    uint8_t eot_count = 0, i;
    int32_t packet_length;
    uint32_t packets_received, errors = 0;
    Ymodem_Status parser_result, recive_result;

    // 发送字符C，以得到发送端响应
    Ymodem_Request(handle);
    for (i = 0; i < 3; i++)
    {
        recive_result = Receive_Packet(handle, packet_data, &packet_length, 5000000, 0);
        WDT_FreeDog();
        if (recive_result == YMODEM_OK && packet_length > 0) // 得到正确起始帧。退出循环
        {
            parser_result = Packet_Parser(handle, packet_data, packet_length, 0); // 解析起始帧
            if (parser_result == YMODEM_OK)
            {
                Ymodem_Acknowledge(handle);
                Ymodem_Request(handle);
                break;
            }
            else
            {
                Ymodem_Cancel(handle);
                return parser_result;
            }
        }
        else if (recive_result == YMODEM_ERR_TIMEOUT)
        { // 超时，继续发字符C
            // handle->hal->delay_ms(1000);
            Ymodem_Request(handle);
        }
        else
        { // 其他错误
            Ymodem_Cancel(handle);
            return recive_result;
        }
    }
    if (i == 3)
    {
        return YMODEM_ERR_CANCELLED;
    }

    // 接收数据帧
    for (packets_received = 1;;)
    {
        recive_result = Receive_Packet(handle, packet_data, &packet_length, 5000000, packets_received);
        WDT_FreeDog();
        if (recive_result == YMODEM_ERR_FRAME) // 帧错误
        {
            Ymodem_NegativeAcknowledge(handle);
        }
        else if (recive_result == YMODEM_OK && packet_length <= 0) // 控制帧
        {
            if (packet_length == -1) // CAN
            {
                return YMODEM_ERR_CANCELLED;
            }
            if (packet_length == 0) // EOT
            {
                eot_count++;
                if (eot_count == 1)
                {
                    Ymodem_NegativeAcknowledge(handle);
                }
                else
                {
                    handle->m_end_flag = 1;
                    Ymodem_Acknowledge(handle);
                    Ymodem_Request(handle);
                }
            }
        }
        else if (recive_result == YMODEM_ERR_TIMEOUT) // 超时
        {
            errors++;
            if (errors >= MAX_ERRORS)
            {
                return YMODEM_ERR_TIMEOUT;
            }
            Ymodem_NegativeAcknowledge(handle);
        }
        else
        {
            parser_result = Packet_Parser(handle, packet_data, packet_length, packets_received);
            if (parser_result == YMODEM_ERR_FRAME)
            {
                Ymodem_NegativeAcknowledge(handle);
            }
            else if (parser_result == YMODEM_OK)
            {
                packets_received++;
                if (handle->m_tmp_len > 0 && handle->m_tmp_len == FLASH_SECTOR_SIZE)
                {
                    handle->m_tmp_len = 0;
                    WDT_FreeDog();
                    handle->hal->storage_write(write_addr, handle->m_file_buf); // 写入剩余不满2048字节的数据
                    WDT_FreeDog();
                    write_addr += FLASH_SECTOR_SIZE;
                }
                Ymodem_Acknowledge(handle);
                if (handle->m_end_flag == 1)
                {
                    if (handle->m_tmp_len > 0)
                    {
                        memset(handle->m_file_buf + handle->m_tmp_len, 0, FLASH_SECTOR_SIZE - handle->m_tmp_len);
                        WDT_FreeDog();
                        handle->hal->storage_write(write_addr, handle->m_file_buf); // 写入剩余不满2048字节的数据
                        WDT_FreeDog();
                    }
                    // Ymodem_Acknowledge(handle);
                    return YMODEM_OK;
                }
            }
            else
            {
                Ymodem_Cancel(handle);
                return parser_result;
            }
        }
    }
}
