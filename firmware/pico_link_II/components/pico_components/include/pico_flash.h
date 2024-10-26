/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:23:14
 * @FilePath: \pico_link_II\components\pico_components\include\pico_flash.h
 * @Description:
 */

#ifndef __PICO_FLASH_H__
#define __PICO_FLASH_H__

#include "pico_headfile.h"

void write_config_to_flash(void);
void read_config_from_flash(void);
void read_default_config_from_flash(void);
void write_encrypt_to_flash(void);
esp_err_t read_encrypt_from_flash(void);

#endif