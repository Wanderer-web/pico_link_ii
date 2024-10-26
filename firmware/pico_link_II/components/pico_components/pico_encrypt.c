/*
 * @Author: Wanderer
 * @Date: 2023-05-16 18:59:22
 * @LastEditors: Wanderer
 * @LastEditTime: 2024-10-26 11:41:32
 * @FilePath: \pico_link_II\components\pico_components\pico_encrypt.c
 * @Description:
 */

#include "pico_encrypt.h"

static const char *TAG = "pico_encrypt";

pico_encrypt_par_t encrypt_par = {
    .a = 0,
    .b = 0,
    .c = 0,
    .d = 0,
};

static esp_err_t pico_decrypt()
{
    uint8_t mac_id[8] = {0};
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac_id));
    ESP_LOGI(TAG, "mac id: %x-%x-%x-%x-%x-%X", mac_id[0], mac_id[1], mac_id[2], mac_id[3], mac_id[4], mac_id[5]);
    uint32_t a = encrypt_par.a;
    uint32_t b = encrypt_par.b;
    uint32_t c = encrypt_par.c;
    uint32_t d = encrypt_par.d;
    uint32_t A = (a | b | c);
    uint32_t B = (a & b & c);
    uint32_t C = 0;
    mac_id[1] = mac_id[1] ^ mac_id[0];
    mac_id[4] = mac_id[4] ^ mac_id[5];
    for (uint8_t i = 1; i <= 4; i++)
    {
        C = (C << 8) + mac_id[i];
    }
    return (((A ^ B ^ C ^ d) == 1626632460) ? ESP_OK : ESP_FAIL);
}

static void pico_encrypt_par_gen()
{
    uint8_t mac_id[8] = {0};
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac_id));
    printf("mac id: %x-%x-%x-%x-%x-%x\n", mac_id[0], mac_id[1], mac_id[2], mac_id[3], mac_id[4], mac_id[5]);
    uint32_t a = esp_random();
    uint32_t b = esp_random();
    uint32_t c = esp_random();
    uint32_t d = 0;
    uint32_t A = (a | b | c);
    uint32_t B = (a & b & c);
    uint32_t C = 0;
    mac_id[1] = mac_id[1] ^ mac_id[0];
    mac_id[4] = mac_id[4] ^ mac_id[5];
    for (uint8_t i = 1; i <= 4; i++)
    {
        C = (C << 8) + mac_id[i];
    }
    d = 1626632460 ^ (A ^ B ^ C);
    printf("gen encrypt par: %d | %d | %d | %d\n", (int)a, (int)b, (int)c, (int)d);
    encrypt_par.a = a;
    encrypt_par.b = b;
    encrypt_par.c = c;
    encrypt_par.d = d;
}

/**
 * @description:
 * @return {*}
 */
void pico_encrypt_check(void)
{
    if (read_encrypt_from_flash() == ESP_OK && pico_decrypt() == ESP_OK)
    {
        ESP_LOGI(TAG, "decrypt success!");
    }
    else
    {
        while (1)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            printf("This chip doesn't support this firmware!\n");
        }
    }
}

void pico_write_encrypt_and_check(void)
{
    pico_encrypt_par_gen();
    write_encrypt_to_flash();
    if (read_encrypt_from_flash() == ESP_OK && pico_decrypt() == ESP_OK)
    {
        SET_RGB_WHITE;
        ESP_LOGI(TAG, "decrypt success!");
    }
    else
    {
        SET_RGB_RED;
        ESP_LOGE(TAG, "encrypt failed!");
    }
}