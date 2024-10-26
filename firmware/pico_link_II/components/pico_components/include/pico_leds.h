/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:25:09
 * @FilePath: \pico_link_II\components\pico_components\include\pico_leds.h
 * @Description:
 */

#ifndef __PICO_PICO_LEDS_H__
#define __PICO_PICO_LEDS_H__

#include "pico_headfile.h"

#define UPLOAD_LED_PIN (0)   // 上行灯
#define DOWNLOAD_LED_PIN (1) // 下行灯

#define UPLOAD_LED_ON (gpio_set_level(UPLOAD_LED_PIN, 0))
#define UPLOAD_LED_OFF (gpio_set_level(UPLOAD_LED_PIN, 1))

#define DOWNLOAD_LED_ON (gpio_set_level(DOWNLOAD_LED_PIN, 0))
#define DOWNLOAD_LED_OFF (gpio_set_level(DOWNLOAD_LED_PIN, 1))

#define RGB_BRIGHTNESS (6)
#define SET_RGB_OFF (set_rgb_color(0, 0, 0))
#define SET_RGB_RED (set_rgb_color(RGB_BRIGHTNESS, 0, 0))
#define SET_RGB_GREEN (set_rgb_color(0, RGB_BRIGHTNESS, 0))
#define SET_RGB_BLUE (set_rgb_color(0, 0, RGB_BRIGHTNESS))
#define SET_RGB_CYAN (set_rgb_color(0, RGB_BRIGHTNESS, RGB_BRIGHTNESS))
#define SET_RGB_YELLOW (set_rgb_color(RGB_BRIGHTNESS, RGB_BRIGHTNESS, 0))
#define SET_RGB_ORANGE (set_rgb_color(RGB_BRIGHTNESS, RGB_BRIGHTNESS / 3, 0))
#define SET_RGB_PURPLE (set_rgb_color(RGB_BRIGHTNESS, 0, RGB_BRIGHTNESS))
#define SET_RGB_WHITE (set_rgb_color(RGB_BRIGHTNESS, RGB_BRIGHTNESS, RGB_BRIGHTNESS))

void set_rgb_color(uint8_t r, uint8_t g, uint8_t b);
void pico_leds_init(void);

#endif