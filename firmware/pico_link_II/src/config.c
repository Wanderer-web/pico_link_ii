/*
 * @Author: Wanderer
 * @Date: 2022-04-26 20:41:06
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:14:19
 * @FilePath: \pico_link_II\src\config.c
 * @Description:
 */

#include "config.h"

#define CONFIG_PIN (19)

#define GPIO_INPUT_PIN_SEL ((1ULL << CONFIG_PIN))

static const char *TAG = "config";

picoConfig_t picoConfig = {
    .protocol = PROTOCOL_UART,
    .uartSpeed = 3000000,
    .SSID = "Best Match",
    .PWD = "88888888",
    .hostIP = "192.168.137.1",
    .port = 2333,
}; // 默认初始化参数

/**
 * @description: 配置模式
 * @param {*}
 * @return {*}
 */
void configMode(void)
{
    setRgbLevel(1, 0, 1); // 指示灯变绿
    ESP_LOGI(TAG, "enter config mode");
    wifi_init_softap();                         // 模块进入AP模式
    ESP_ERROR_CHECK(startWebServer("/spiffs")); // 启动web服务器
    while (1)
        ;
}

/**
 * @description: 配置初始化
 * @param {*}
 * @return {*}
 */
void configInit(void)
{
    readFlashConfig(); // 读取参数
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
            configMode();
        }
    }
}
