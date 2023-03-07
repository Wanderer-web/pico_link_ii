/***
 * @Author: Wanderer
 * @Date: 2022-04-26 20:41:24
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:10:57
 * @FilePath: \pico_link_II\include\config.h
 * @Description:
 */

#ifndef _CONFIG_H__
#define _CONFIG_H__

#include "headfile.h"

#define PROTOCOL_UART (0)
#define PROTOCOL_SPI (1)

typedef struct
{
    uint8_t protocol;
    uint32_t uartSpeed;
    char SSID[32]; /**< SSID of the AP. Null terminated string. */
    char PWD[64];  /**< Password of the AP. Null terminated string. */
    char hostIP[16];
    uint16_t port;
} picoConfig_t;

extern picoConfig_t picoConfig;

void configInit(void);

#endif
