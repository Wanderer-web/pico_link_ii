/***
 * @Author: Wanderer
 * @Date: 2022-04-26 20:43:14
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:13:43
 * @FilePath: \pico_link_II\include\web_server.h
 * @Description:
 */

#ifndef _WEB_SERVER_H__
#define _WEB_SERVER_H__

#include "headfile.h"

esp_err_t startWebServer(const char *base_path);

#endif
