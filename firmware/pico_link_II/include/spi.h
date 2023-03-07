/***
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:12:27
 * @FilePath: \pico_link_II\include\spi.h
 * @Description:
 */

#ifndef _SPI_H__
#define _SPI_H__

#include "headfile.h"

#define SPI_RX_BUF_SIZE (30000)

void spiInit(void);
void spiPollRecv(char *spiRecvDataPtr, int *spiRecvDataLengthPtr);

#endif