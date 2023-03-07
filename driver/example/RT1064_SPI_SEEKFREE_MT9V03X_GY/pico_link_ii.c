// 版本1.0
#include "pico_link_ii.h"

//------------用户替换函数需要包含的头文件-----------------------
#include "zf_common_headfile.h" // 逐飞RT1064库版本3.3.3

//--------------------UART 宏定义------------------------------
#define CONNECT_CTS_PIN (1) // 是否接了流控引脚

// 读取CTS脚GPIO电平, 需要自己找函数定义, 该函数要有返回值, 0是低电平, 1是高电平
#define READ_CTS gpio_get_level((D26))

// UART发送多个字节，需要自己找函数定义, pBuff:发送内容首地址, len:发送长度
#define UART_SEND_BYTES(pBuff, len) uart_write_buffer((UART_8), (pBuff), (len))

//----------------------SPI 宏定义-----------------------------
// SPI发送多个字节，需要自己找函数定义, pBuff:发送内容首地址, len:发送长度
#define SPI_SEND_BYTES(pBuff, len) spi_write_8bit_array((SPI_4), (pBuff), (len))

//----------------------发送缓冲区------------------------------
#define BUFFER_SIZE (25000) // 发送缓冲区大小，建议略大于图像字节数，太大会占用过多内存
static pico_uint8 sendBuffer[BUFFER_SIZE] = {0};

/**
 * @description: Pico Link II 串口发送多个字节（硬件协议端）
 * @param {pico_uint8} *pBuff 数据指针
 * @param {pico_uint32} len 数据长度
 * @return {*}
 */
void BaseSendBytes_Uart(pico_uint8 *pBuff, pico_uint32 len)
{
#if CONNECT_CTS_PIN
#define UART_FIFO_LENGTH (128) // 接收端UART FIFO长度
    while (len > UART_FIFO_LENGTH)
    {
        while (READ_CTS)
            ; // 如果CTS为低电平，则继续发送数据
        UART_SEND_BYTES(pBuff, UART_FIFO_LENGTH);
        pBuff += UART_FIFO_LENGTH; // 地址偏移
        len -= UART_FIFO_LENGTH;   // 数量
    }
    while (READ_CTS)
        ;                        // 如果CTS为低电平，则继续发送数据
    UART_SEND_BYTES(pBuff, len); // 发送最后的数据
#else
    UART_SEND_BYTES(pBuff, len);
#endif
}

/**
 * @description:Pico Link II 串口发送多个字节
 * @param {pico_uint8} *data 数据指针
 * @param {pico_uint32} length 数据长度
 * @return {*}
 */
void picoSendBytes_Uart(pico_uint8 *data, pico_uint32 length)
{
    pico_uint16 len = (pico_uint16)(length);
    BaseSendBytes_Uart((pico_uint8 *)(&len), 2); // 小端模式通知发送长度
    BaseSendBytes_Uart(data, length);            // 发送数据
}

/**
 * @description:Pico Link II SPI发送多个字节
 * @param {pico_uint8} *data 数据指针
 * @param {pico_uint32} length 数据长度,需要为4的倍数
 * @return {*}
 */
void picoSendBytes_Spi(pico_uint8 *data, pico_uint32 length)
{
    if (length % 4 != 0)
        return;
    else
        SPI_SEND_BYTES(data, length);
}

/**
 * @description: Pico Link II 串口发送图像到致用上位机
 * @param {pico_uint8} *pImage 图像指针
 * @param {pico_uint32} imageH 图像高度
 * @param {pico_uint32} imageW 图像宽度
 * @return {*}
 */
void ipcSendImg_Uart(pico_uint8 *pImage, pico_uint32 imageH, pico_uint32 imageW)
{
    pico_uint16 len = (pico_uint16)(imageH * imageW + 6);
    BaseSendBytes_Uart((pico_uint8 *)(&len), 2); // 小端模式通知发送长度
    BaseSendBytes_Uart((pico_uint8 *)"CSU", 3);  // 发送帧头
    BaseSendBytes_Uart(pImage, imageH * imageW); // 发送图像数据
    BaseSendBytes_Uart((pico_uint8 *)"USC", 3);  // 发送帧尾
}

/**
 * @description: Pico Link II SPI发送图像到致用上位机
 * @param {pico_uint8} *pImage 图像指针
 * @param {pico_uint32} imageH 图像高度
 * @param {pico_uint32} imageW 图像宽度
 * @return {*}
 */
void ipcSendImg_Spi(pico_uint8 *pImage, pico_uint32 imageH, pico_uint32 imageW)
{
    pico_uint8 overLength = 4 - (6 + imageH * imageW) % 4;
    memcpy((sendBuffer + 0), "CSU", 3);
    memcpy((sendBuffer + 3), pImage, imageH * imageW);
    memcpy((sendBuffer + 3 + imageH * imageW), "USC", 3);
    memset((sendBuffer + 6 + imageH * imageW), '\0', overLength);
    picoSendBytes_Spi(sendBuffer, (6 + imageH * imageW + overLength));
}