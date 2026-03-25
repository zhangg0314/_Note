/**
 **************************************************************************
 * @file     common.c
 * @brief    common function program
 **************************************************************************
 */
#include "common.h"
/**
 * @brief  反转字符串函数
 * @param  len: 字符串长度
 *         str：字符串
 * @retval None.
 */
void reverse(uint8_t *str, uint32_t len)
{
    uint32_t i = 0, j = len - 1;
    uint8_t temp;
    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
/**
 * @brief   整形数据转到字符串
 * @param  intnum: 数据
 *         str：转换为的字符串
 * @retval None.
 */
void Int2Str(uint8_t *str, int32_t intnum)
{
    uint32_t i = 0;
    uint8_t is_negative = 0;
    uint32_t num;

    // 处理零的情况
    if (intnum == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // 处理负数
    if (intnum < 0)
    {
        is_negative = 1;
        // 转换为正数处理，注意处理最小负数的特殊情况
        num = (uint32_t)(intnum == INT32_MIN ? -(intnum + 1) + 1 : -intnum);
    }
    else
    {
        num = (uint32_t)intnum;
    }

    // 提取每一位数字并存储（逆序）
    while (num != 0)
    {
        uint32_t rem = num % 10;
        str[i++] = (uint8_t)(rem + '0');
        num = num / 10;
    }

    // 如果是负数，添加负号
    if (is_negative)
    {
        str[i++] = '-';
    }

    // 添加字符串结束符
    str[i] = '\0';

    // 反转字符串得到正确的顺序
    reverse(str, i);
}
/**
 * @brief  字符串转到数据
 * @param  inputstr: 需转换的字符串
 *           intnum：转好的数据
 * @retval 0：正确
 *         -1：错误
 */
int32_t Str2UInt(uint8_t *inputstr, uint32_t *uintnum)
{
    uint8_t digit;
    uint32_t result = 0;
    bool has_digits = false;

    // 检查输入指针有效性
    if (inputstr == NULL || uintnum == NULL)
    {
        return -1; // 无效输入
    }

    // 检查空字符串
    if (*inputstr == '\0')
    {
        return -1; // 空字符串
    }

    // 跳过前导空格
    while (*inputstr == ' ')
    {
        inputstr++;
    }

    // 处理数字部分
    while (*inputstr != '\0')
    {
        // 检查是否为有效数字
        if (*inputstr < '0' || *inputstr > '9')
        {
            return -1; // 包含无效字符
        }

        has_digits = true;

        // 计算当前数字值
        digit = *inputstr - '0';

        // 检查溢出：如果当前结果大于UINT32_MAX/10，或者等于且下一位会溢出
        if (result > UINT32_MAX / 10 ||
            (result == UINT32_MAX / 10 && digit > UINT32_MAX % 10))
        {
            return -1; // 数值溢出
        }

        // 累加结果
        result = result * 10 + digit;
        inputstr++;
    }

    // 检查是否有有效数字
    if (!has_digits)
    {
        return -1; // 无有效数字
    }

    // 存储结果
    *uintnum = result;
    return 0; // 转换成功
}
