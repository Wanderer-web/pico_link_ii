/*
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:14:31
 * @FilePath: \pico_link_II\src\flash.c
 * @Description:
 */

#include "flash.h"

static const char *TAG = "flash";

/**
 * @description: 向Flash写入参数
 * @param {*}
 * @return {*}
 */
void writeFlashConfig(void)
{
    // ESP_LOGI(TAG, "write flash");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "open done\n");
        ESP_ERROR_CHECK(nvs_set_u8(my_handle, "protocol", picoConfig.protocol));
        ESP_ERROR_CHECK(nvs_set_u32(my_handle, "uartSpeed", picoConfig.uartSpeed));
        ESP_ERROR_CHECK(nvs_set_str(my_handle, "SSID", picoConfig.SSID));
        ESP_ERROR_CHECK(nvs_set_str(my_handle, "PWD", picoConfig.PWD));
        ESP_ERROR_CHECK(nvs_set_str(my_handle, "hostIP", picoConfig.hostIP));
        ESP_ERROR_CHECK(nvs_set_u16(my_handle, "port", picoConfig.port));
        ESP_LOGI(TAG, "write done\n");
    }
    // Close
    nvs_close(my_handle);
}

/**
 * @description: 读取Flash设置参数
 * @param {*}
 * @return {*}
 */
void readFlashConfig(void)
{
    ESP_LOGI(TAG, "read flash");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "open done\n");
        err = nvs_get_u8(my_handle, "protocol", &picoConfig.protocol);
        if (err == ESP_OK)
        {
            ESP_ERROR_CHECK(nvs_get_u32(my_handle, "uartSpeed", &picoConfig.uartSpeed));
            size_t length = 32;
            ESP_ERROR_CHECK(nvs_get_str(my_handle, "SSID", picoConfig.SSID, &length));
            length = 64;
            ESP_ERROR_CHECK(nvs_get_str(my_handle, "PWD", picoConfig.PWD, &length));
            length = 16;
            ESP_ERROR_CHECK(nvs_get_str(my_handle, "hostIP", picoConfig.hostIP, &length));
            ESP_ERROR_CHECK(nvs_get_u16(my_handle, "port", &picoConfig.port));
        }
        else if (err == ESP_ERR_NVS_NOT_FOUND) // 值没有初始化
        {
            ESP_LOGE(TAG, "The value is not initialized yet!\n");
            ESP_ERROR_CHECK(nvs_set_u8(my_handle, "protocol", picoConfig.protocol));
            ESP_ERROR_CHECK(nvs_set_u32(my_handle, "uartSpeed", picoConfig.uartSpeed));
            ESP_ERROR_CHECK(nvs_set_str(my_handle, "SSID", picoConfig.SSID));
            ESP_ERROR_CHECK(nvs_set_str(my_handle, "PWD", picoConfig.PWD));
            ESP_ERROR_CHECK(nvs_set_str(my_handle, "hostIP", picoConfig.hostIP));
            ESP_ERROR_CHECK(nvs_set_u16(my_handle, "port", picoConfig.port));
            ESP_LOGI(TAG, "initial write done\n");
        }
        else
        {
            ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
        }
        ESP_LOGI(TAG, "read done\n");
    }
    // Close
    nvs_close(my_handle);
}
