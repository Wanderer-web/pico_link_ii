/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:27:08
 * @FilePath: \pico_link_II\components\pico_components\include\pico_wifi_sta.h
 * @Description:
 */

#ifndef __PICO_WIFI_STA_H__
#define __PICO_WIFI_STA_H__

#include "pico_headfile.h"

extern uint8_t is_sta_connect;

void pico_wifi_init_sta(void);

#endif
