/***
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:12:03
 * @FilePath: \pico_link_II\include\led.h
 * @Description:
 */

#ifndef _LED_H__
#define _LED_H__

#include "headfile.h"

void ledInit(void);
void setRgbLevel(uint8_t rLevel, uint8_t gLevel, uint8_t bLevel);
void setTranLedLevel(uint8_t level);

#endif