/*
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-10-21 17:22:13
 * @FilePath: \pico_link_II\components\pico_components\pico_flash.c
 * @Description:
 */

#include "pico_flash.h"

static const char *TAG = "pico_flash";

/**
 * @description: 向flash写入配置参数
 * @param {*}
 * @return {*}
 */
void write_config_to_flash(void)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "open done");
        ESP_ERROR_CHECK(nvs_set_blob(my_handle, "pico_config", &pico_config, sizeof(pico_config_t)));
        ESP_LOGI(TAG, "write done");
    }
    // Close
    nvs_close(my_handle);
}

/**
 * @description: 从flash读取配置参数
 * @param {*}
 * @return {*}
 */
void read_config_from_flash(void)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "open done");
        uint32_t config_version = 0;
        ESP_LOGI(TAG, "flash read start");
        err = nvs_get_u32(my_handle, "config_version", &config_version); // 读取配置版本信息
        if (err == ESP_OK)
        {
            if (config_version == PICO_CONFIG_VERSION)
            {
                size_t length = sizeof(pico_config_t);
                ESP_ERROR_CHECK(nvs_get_blob(my_handle, "pico_config", &pico_config, &length));
            }
            else // 不是最新配置参数
            {
                ESP_LOGI(TAG, "need overwriting");
                ESP_ERROR_CHECK(nvs_set_u32(my_handle, "config_version", PICO_CONFIG_VERSION));
                ESP_ERROR_CHECK(nvs_set_blob(my_handle, "default_config", &pico_config, sizeof(pico_config_t)));
                ESP_ERROR_CHECK(nvs_set_blob(my_handle, "pico_config", &pico_config, sizeof(pico_config_t)));
                ESP_LOGI(TAG, "overwrite done");
            }
        }
        else if (err == ESP_ERR_NVS_NOT_FOUND) // 值没有初始化
        {
            ESP_LOGI(TAG, "the value is not initialized");
            ESP_ERROR_CHECK(nvs_set_u32(my_handle, "config_version", PICO_CONFIG_VERSION));
            ESP_ERROR_CHECK(nvs_set_blob(my_handle, "default_config", &pico_config, sizeof(pico_config_t)));
            ESP_ERROR_CHECK(nvs_set_blob(my_handle, "pico_config", &pico_config, sizeof(pico_config_t)));
            ESP_LOGI(TAG, "initial done");
        }
        else
        {
            ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
        }
        ESP_LOGI(TAG, "flash read done");
    }
    // Close
    nvs_close(my_handle);
}

/**
 * @description: 从flash读取出厂配置参数
 * @param {*}
 * @return {*}
 */
void read_default_config_from_flash(void)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "open done");
        ESP_LOGI(TAG, "flash read start");
        if (err == ESP_OK)
        {
            size_t length = sizeof(pico_config_t);
            ESP_ERROR_CHECK(nvs_get_blob(my_handle, "default_config", &pico_config, &length));
        }
        else
        {
            ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
        }
        ESP_LOGI(TAG, "flash read done");
    }
    // Close
    nvs_close(my_handle);
}

/**
 * @description: 向flash写入加密密钥
 * @return {*}
 */
void write_encrypt_to_flash(void)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "open done");
        ESP_ERROR_CHECK(nvs_set_blob(my_handle, "encrypt_par", &encrypt_par, sizeof(pico_encrypt_par_t)));
        ESP_LOGI(TAG, "write done");
    }
    // Close
    nvs_close(my_handle);
}

/**
 * @description: 从flash读取加密密钥
 * @return {*}
 */
esp_err_t read_encrypt_from_flash(void)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        err = ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG, "open done");
        size_t length = sizeof(pico_encrypt_par_t);
        ESP_LOGI(TAG, "flash read start");
        err = nvs_get_blob(my_handle, "encrypt_par", &encrypt_par, &length);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
            err = ESP_FAIL;
        }
        ESP_LOGI(TAG, "flash read done");
    }
    // Close
    nvs_close(my_handle);
    return err;
}
