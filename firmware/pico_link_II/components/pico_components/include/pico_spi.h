/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:29:27
 * @FilePath: \pico_link_II\components\pico_components\include\pico_spi.h
 * @Description:
 */

#ifndef __PICO_SPI_H__
#define __PICO_SPI_H__

#include "pico_headfile.h"

void pico_spi_init(void);
int spi_poll_recv(char **spi_recv_data_ptr);
void pico_spi_hd_init(void);
int spi_hd_poll_recv(char **spi_recv_data_ptr);
BaseType_t spi_send_queue(char *spi_send_data, int len);

#endif