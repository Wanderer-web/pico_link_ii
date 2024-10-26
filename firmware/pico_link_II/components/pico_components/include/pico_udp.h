/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:31:26
 * @FilePath: \pico_link_II\components\pico_components\include\pico_udp.h
 * @Description:
 */

#ifndef __PICO_UDP_H__
#define __PICO_UDP_H__

#include "pico_headfile.h"

void pico_udp_client_init(void);
void udp_client_send(char *udp_send_data, int udp_send_data_len);
void pico_udp_server_init(void);
void udp_server_send(char *udp_send_data, int udp_send_data_len);

#endif
