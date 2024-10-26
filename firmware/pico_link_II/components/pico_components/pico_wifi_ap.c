/*
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-11-23 21:27:30
 * @FilePath: \pico_link_II\components\pico_components\pico_wifi_ap.c
 * @Description:
 */

#include "pico_wifi_ap.h"

#define CONFIG_WIFI_SSID "Pico Link II Config"
#define CONFIG_WIFI_PWD "88888888"
#define CONFIG_WIFI_CHANNEL 1
#define CONFIG_MAX_STA_CONN 1

static const char *TAG = "pico_wifi_ap";

static void ap_set_static_ip(esp_netif_t *netif)
{
    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
    ip_info.ip.addr = ipaddr_addr(pico_config.local_ip);
    ip_info.netmask.addr = ipaddr_addr(pico_config.local_netmask);
    ip_info.gw.addr = ipaddr_addr(pico_config.local_gw);
    ESP_ERROR_CHECK(esp_netif_dhcps_stop(netif));
    if (esp_netif_set_ip_info(netif, &ip_info) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set ip info");
        SET_RGB_CYAN;
        while (1)
            ; // 卡死在这
    }
    ESP_ERROR_CHECK(esp_netif_dhcps_start(netif));
    ESP_LOGI(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", pico_config.local_ip, pico_config.local_netmask, pico_config.local_gw);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void pico_wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif != NULL);

    ap_set_static_ip(ap_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_country_t country = {
        .cc = "CN",
        .schan = 1,
        .nchan = 13,
        .max_tx_power = 20,
        .policy = WIFI_COUNTRY_POLICY_AUTO,
    };
    ESP_ERROR_CHECK(esp_wifi_set_country(&country));
    ESP_LOGI(TAG, "Done with set_country");

    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(pico_config.AP_SSID),
            .channel = pico_config.wifi_ap_channel,
            .max_connection = pico_config.wifi_ap_max_conn,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    memcpy(wifi_config.ap.ssid, pico_config.AP_SSID, PICO_SSID_LENGTH);
    if (strlen(pico_config.AP_PWD) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    else
    {
        memcpy(wifi_config.ap.password, pico_config.AP_PWD, PICO_PWD_LENGTH);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d", pico_config.AP_SSID, pico_config.AP_PWD, pico_config.wifi_ap_channel);

    SET_RGB_PURPLE; // 指示灯变紫
}

void pico_wifi_init_softap_config(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CONFIG_WIFI_SSID,
            .ssid_len = strlen(CONFIG_WIFI_SSID),
            .channel = CONFIG_WIFI_CHANNEL,
            .password = CONFIG_WIFI_PWD,
            .max_connection = CONFIG_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    if (strlen(CONFIG_WIFI_PWD) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d", CONFIG_WIFI_SSID, CONFIG_WIFI_PWD, CONFIG_WIFI_CHANNEL);

    SET_RGB_GREEN; // 指示灯变绿
}