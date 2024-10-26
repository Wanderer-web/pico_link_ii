/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:26:01
 * @FilePath: \pico_link_II\components\pico_components\include\pico_headfile.h
 * @Description:
 */

#ifndef __PICO_HEADFILE_H__
#define __PICO_HEADFILE_H__

// 官方头文件

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "esp_vfs_fat.h"
#include "esp_timer.h"
#include "esp_random.h"

#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/apps/netbiosns.h"
#include "lwip/apps/mdns.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/spi_slave.h"
#include "driver/spi_slave_hd.h"
#include "driver/ledc.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/sdmmc_host.h"

#include "cJSON.h"
#include "sdmmc_cmd.h"

#include "led_strip.h"

// 用户头文件
#include "pico_config.h"
#include "pico_leds.h"
#include "pico_udp.h"
#include "pico_tcp.h"
#include "pico_wifi_sta.h"
#include "pico_wifi_ap.h"
#include "pico_web_server.h"
#include "pico_flash.h"
#include "pico_uart.h"
#include "pico_spi.h"
#include "pico_encrypt.h"

typedef struct
{
    int len;    // 数据长度
    char *data; // 数据头指针
} pico_queue_message_t;

#define PICO_QUEUE_PKG_LENGTH (4096) // 消息队列传递数据最大长度

#define PICO_MASTER_TASK_STACK_SIZE (2048) // 主模式任务堆栈大小
#define PICO_MASTER_TASK_PRIORITY (9)      // 主模式任务优先级
#define PICO_MASTER_TASK_CORE (0)          // 主模式任务执行核
// ->SOCKET
#define PICO_SOCKET_QUEUE_SIZE (16) // SOCKET发送消息队列长度

#define PICO_SLAVE_TASK_STACK_SIZE (2048) // 从模式任务堆栈大小
#define PICO_SLAVE_TASK_PRIORITY (10)     // 从模式任务优先级
#define PICO_SLAVE_TASK_CORE (0)          // 从模式任务执行核
// UART->
#define PICO_UART_RX_PKG_NUM (16) // UART接收环形缓存长度相对于[PICO_QUEUE_PKG_LENGTH]的倍数
// SPI->
#define PICO_SPI_QUEUE_SIZE (6) // SPI传输事务队列长度, 即SPI接收缓存的个数
#define PICO_SPI_RX_PKG_NUM (4) // SPI单次传输事务最大接收长度相对于[PICO_QUEUE_PKG_LENGTH]的倍数
// UART/SPI<-
#define PICO_TX_BUF_NUM_UART (PICO_SPI_QUEUE_SIZE + 0)   // 双向发送发送缓存个数 (UART)
#define PICO_TX_BUF_NUM_FD_SPI (PICO_SPI_QUEUE_SIZE + 3) // 双向发送发送缓存个数 (全双工SPI)
#define PICO_TX_BUF_NUM_HD_SPI (PICO_SPI_QUEUE_SIZE + 1) // 双向发送发送缓存个数 (半双工SPI)
#define PICO_TX_BUF_NUM_MAX (PICO_TX_BUF_NUM_FD_SPI)     // 双向发送发送缓存最大个数
#define PICO_TX_BUF_SIZE (4096)                          // 双向发送发送缓存大小
#define PICO_TX_QUEUE_SIZE (PICO_SPI_QUEUE_SIZE)         // 双向发送发送消息队列长度

#endif
