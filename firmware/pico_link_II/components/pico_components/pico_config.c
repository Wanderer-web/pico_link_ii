/*
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-11-21 12:59:10
 * @FilePath: \pico_link_II\components\pico_components\pico_config.c
 * @Description:
 */

#include "pico_config.h"

#define CONFIG_PIN (8)

#define GPIO_INPUT_PIN_SEL ((1ULL << CONFIG_PIN))

static const char *TAG = "pico_config";

pico_config_t pico_config = {
    .in_method = IN_METHOD_UART,
    .uart_config = {
        .uart_rate = 3000000,
        .uart_data_bit = 3,
        .uart_parity = 0,
        .uart_stop_bit = 0,
    },
    .spi_clk_mode = 3,
    .spi_bus_mode = SPI_BUS_FD_1bit,
    .out_method = OUT_METHOD_UDP,
    .wifi_mode = PICO_WIFI_MODE_STA,
    .wifi_ap_max_conn = 1,
    .wifi_ap_channel = 1,
    .STA_SSID = "Best Match",
    .STA_PWD = "88888888",
    .AP_SSID = "Pico AP",
    .AP_PWD = "88888888",
    .two_way_en = 0,
    .work_mode = WORK_MODE_CLIENT,
    .remote_ip = "192.168.137.1",
    .remote_port = 2333,
    .local_gw = "192.168.4.1",
    .local_netmask = "255.255.255.0",
    .local_ip = "192.168.4.2",
    .local_port = 2333,
}; // 默认初始化参数

/**
 * @description: 重置参数
 * @return {*}
 */
void pico_reset_config(void)
{
    read_default_config_from_flash(); // 从flash读取默认参数
    write_config_to_flash();          // 将默认参数写入flash
}

/**
 * @description: 配置模式
 * @param {*}
 * @return {*}
 */
static void pico_enter_config_mode(void)
{
    ESP_LOGI(TAG, "enter config mode");
    pico_wifi_init_softap_config();            // 模块进入AP模式
    ESP_ERROR_CHECK(start_web_server("/www")); // 启动web服务器
    while (1)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        UPLOAD_LED_OFF;
        DOWNLOAD_LED_ON;
        vTaskDelay(500 / portTICK_PERIOD_MS);
        UPLOAD_LED_ON;
        DOWNLOAD_LED_OFF;
    }
}

/**
 * @description: 配置初始化
 * @param {*}
 * @return {*}
 */
void pico_config_init(void)
{
    read_config_from_flash(); // 读取参数
    gpio_config_t io_conf = {};
    // 输入引脚配置
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    if (gpio_get_level(CONFIG_PIN) == 0)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (gpio_get_level(CONFIG_PIN) == 0)
        {
            pico_enter_config_mode();
        }
    }
}
