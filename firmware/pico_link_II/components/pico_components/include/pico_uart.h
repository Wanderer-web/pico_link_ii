/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:29:07
 * @FilePath: \pico_link_II\components\pico_components\include\pico_uart.h
 * @Description:
 */

#ifndef __PICO_UART_H__
#define __PICO_UART_H__

#include "pico_headfile.h"

void pico_uart_init(void);
int uart_poll_recv(char *uart_recv_data);
int uart_poll_recv_256B(char *uart_recv_data);
BaseType_t uart_send_queue(char *uart_send_data, int len);

#endif