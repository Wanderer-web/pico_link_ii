/***
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:12:48
 * @FilePath: \pico_link_II\include\uart.h
 * @Description:
 */

#ifndef _UART_H__
#define _UART_H__

#include "headfile.h"

#define UART_RX_BUF_SIZE (25000)

void uartRecvInit(void);
void uartPollRecv(char *uartRecvDataPtr, int *uartRecvDataLengthPtr);

#endif