/***
 * @Author: Wanderer
 * @Date: 2023-03-11 09:57:19
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-03-11 09:59:16
 * @FilePath: \pico_link_II\include\tcp.h
 * @Description:
 */

#ifndef _TCP_H__
#define _TCP_H__

#include "headfile.h"

#define TCP_RX_BUF_SIZE (128)

void tcpInit(void);
void tcpClientSend(char *tcpSendDataPtr, int tcpSendDataLength);

#endif
