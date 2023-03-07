#ifndef _PICO_LINK_II_H__
#define _PICO_LINK_II_H__

// 数据类型声明
typedef unsigned char pico_uint8;
typedef unsigned short int pico_uint16;
typedef unsigned long int pico_uint32;

void BaseSendBytes_Uart(pico_uint8 *pBuff, pico_uint32 len);
void picoSendBytes_Uart(pico_uint8 *data, pico_uint32 length);
void picoSendBytes_Spi(pico_uint8 *data, pico_uint32 length);
void ipcSendImg_Uart(pico_uint8 *pImage, pico_uint32 imageH, pico_uint32 imageW);
void ipcSendImg_Spi(pico_uint8 *pImage, pico_uint32 imageH, pico_uint32 imageW);

#endif
