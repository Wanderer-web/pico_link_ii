/*
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:24:42
 * @FilePath: \pico_link_II\components\pico_components\pico_leds.c
 * @Description:
 */

#include "pico_leds.h"

static const char *TAG = "pico_leds";

#define GPIO_OUTPUT_PIN_SEL ((1ULL << UPLOAD_LED_PIN) | (1ULL << DOWNLOAD_LED_PIN))

static led_strip_handle_t led_strip;

// GPIO assignment
#define LED_STRIP_BLINK_GPIO (19)

// LED numbers in the strip
#define LED_STRIP_LED_NUMBERS 1

// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)

/**
 * @description: 上行灯，下行灯初始化
 * @return {*}
 */
static void upload_download_leds_init(void)
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
    UPLOAD_LED_OFF;
    DOWNLOAD_LED_OFF;
    ESP_LOGI(TAG, "init upload and download leds");
}

/**
 * @description: 设置RGB灯颜色
 * @param {uint8_t} r 红色
 * @param {uint8_t} g 绿色
 * @param {uint8_t} b 蓝色
 * @return {*}
 */
void set_rgb_color(uint8_t r, uint8_t g, uint8_t b)
{
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, r, g, b));
    /* Refresh the strip to send data */
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}

/**
 * @description: RGB灯初始化
 * @return {*}
 */
static void rgb_led_init(void)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_BLINK_GPIO,   // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_LED_NUMBERS,        // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_WS2812,            // LED strip model
        .flags.invert_out = false,                // whether to invert the output signal
    };

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .flags.with_dma = false,               // DMA feature is available on ESP target like ESP32-S3
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    ESP_LOGI(TAG, "init ws2812 rgb led");
}

/**
 * @description: 灯初始化
 * @return {*}
 */
void pico_leds_init(void)
{
    rgb_led_init();
    upload_download_leds_init();
}