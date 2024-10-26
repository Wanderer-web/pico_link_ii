/***
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-11-18 20:20:18
 * @FilePath: \pico_link_II\components\pico_components\include\pico_config.h
 * @Description:
 */

#ifndef __PICO_CONFIG_H__
#define __PICO_CONFIG_H__

#include "pico_headfile.h"

#define IN_METHOD_UART (0)
#define IN_METHOD_SPI (1)

#define SPI_BUS_FD_1bit (0)
#define SPI_BUS_HD_1_2bit (1)

#define OUT_METHOD_UDP (0)
#define OUT_METHOD_TCP (1)

#define PICO_WIFI_MODE_STA (0)
#define PICO_WIFI_MODE_AP (1)

#define WORK_MODE_CLIENT (0)
#define WORK_MODE_SERVER (1)

#define PICO_SSID_LENGTH (32)
#define PICO_PWD_LENGTH (64)
#define PICO_IP_LENGTH (16)
#define PICO_NETMASK_LENGTH (16)

#define PICO_CONFIG_VERSION (uint32_t)(0x11211258)

typedef struct
{
    uint32_t uart_rate;
    uint8_t uart_data_bit;
    uint8_t uart_parity;
    uint8_t uart_stop_bit;
} __attribute__((packed)) pico_uart_config_t;

typedef struct
{
    uint8_t in_method;
    pico_uart_config_t uart_config;
    uint8_t spi_clk_mode;
    uint8_t spi_bus_mode;
    uint8_t out_method;
    uint8_t wifi_mode; // sta / ap
    uint8_t wifi_ap_max_conn;
    uint8_t wifi_ap_channel;
    char STA_SSID[PICO_SSID_LENGTH]; // SSID of the STA. Null terminated string.
    char STA_PWD[PICO_PWD_LENGTH];   // Password of the STA. Null terminated string.
    char AP_SSID[PICO_SSID_LENGTH];  // SSID of the AP. Null terminated string.
    char AP_PWD[PICO_PWD_LENGTH];    // Password of the AP. Null terminated string.
    uint8_t two_way_en;
    uint8_t work_mode; // client / server
    char remote_ip[PICO_IP_LENGTH];
    uint16_t remote_port;
    char local_gw[PICO_IP_LENGTH];
    char local_netmask[PICO_NETMASK_LENGTH];
    char local_ip[PICO_IP_LENGTH];
    uint16_t local_port;
} __attribute__((packed)) pico_config_t;

extern pico_config_t pico_config;

void pico_reset_config(void);
void pico_config_init(void);

#endif
