/***
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:13:24
 * @FilePath: \pico_link_II\include\udp.h
 * @Description:
 */

#ifndef _UDP_H__
#define _UDP_H__

#include "headfile.h"

void udpInit(void);
void udpClientSend(char *udpSendDataPtr, int udpSendDataLength);

#endif
