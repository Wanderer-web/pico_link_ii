/*
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-03-05 15:47:12
 * @FilePath: \pico_link_II\src\udp.c
 * @Description:
 */

#include "udp.h"

static const char *TAG = "udp client";

int sock;
struct sockaddr_in dest_addr;

/**
 * @description: udp client初始化
 * @param {*}
 * @return {*}
 */
void udpInit(void)
{
    int addr_family = 0;
    int ip_protocol = 0;
    dest_addr.sin_addr.s_addr = inet_addr(picoConfig.hostIP);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(picoConfig.port);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        while (1)
            ; // 卡死在这
    }
    ESP_LOGI(TAG, "Socket created, sending to %s:%d", picoConfig.hostIP, picoConfig.port);
    setRgbLevel(1, 1, 0);
}

/**
 * @description:  udp client任务
 * @param {void} *pvParameters
 * @return {*}
 */
void udpClientSend(char *udpSendDataPtr, int udpSendDataLength)
{
    int err = sendto(sock, udpSendDataPtr, udpSendDataLength, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            setRgbLevel(0, 1, 1);
            shutdown(sock, 0);
            close(sock);
            udpInit(); // 重启
        }
    }
}