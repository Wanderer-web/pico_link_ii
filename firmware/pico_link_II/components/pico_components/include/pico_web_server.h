/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:21:30
 * @FilePath: \pico_link_II\components\pico_components\include\pico_web_server.h
 * @Description:
 */

#ifndef __PICO_WEB_SERVER_H__
#define __PICO_WEB_SERVER_H__

#include "pico_headfile.h"

esp_err_t start_web_server(const char *base_path);

#endif
