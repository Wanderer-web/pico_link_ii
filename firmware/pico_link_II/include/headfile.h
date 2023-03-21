/***
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:11:45
 * @FilePath: \pico_link_II\include\headfile.h
 * @Description:
 */

#ifndef _HEADFILE_H
#define _HEADFILE_H

// 官方驱动头文件
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/spi_slave.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

// 用户头文件
#include "led.h"
#include "tcp.h"
#include "udp.h"
#include "wifi_sta.h"
#include "wifi_ap.h"
#include "uart.h"
#include "spi.h"
#include "flash.h"
#include "config.h"
#include "web_server.h"

#endif
