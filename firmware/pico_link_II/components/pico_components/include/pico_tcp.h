/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:31:12
 * @FilePath: \pico_link_II\components\pico_components\include\pico_tcp.h
 * @Description:
 */

#ifndef __PICO_TCP_H__
#define __PICO_TCP_H__

#include "pico_headfile.h"

void pico_tcp_client_init(void);
void tcp_client_send(char *tcp_send_data, int tcp_send_data_len);
void pico_tcp_server_init(void);
void tcp_server_send(char *tcp_send_data, int tcp_send_data_len);

#endif
