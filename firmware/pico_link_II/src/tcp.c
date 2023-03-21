/*
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-03-21 20:03:31
 * @FilePath: \pico_link_II\src\tcp.c
 * @Description:
 */

#include "tcp.h"

static const char *TAG = "tcp client";

static int sock;
static struct sockaddr_in dest_addr;

void tcpClientRecvTask(void *arg)
{
    char rx_buffer[TCP_RX_BUF_SIZE];
    int len;
    while (1)
    {
        vTaskDelay(300 / portTICK_PERIOD_MS);
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len > 0)
        {
            uart_write_bytes(UART_NUM_0, rx_buffer, len);
        }
        // Error occurred during receiving
        else if (len < 0)
        {
            ESP_LOGE(TAG, "recv failed: errno %d", errno);
            if (sock != -1)
            {
                setRgbLevel(0, 1, 1);
                shutdown(sock, 0);
                close(sock);
                tcpInit(); // 重启
            }
        }
    }
}

/**
 * @description: tcp client初始化
 * @param {*}
 * @return {*}
 */
void tcpInit(void)
{
    int addr_family = 0;
    int ip_protocol = 0;
    inet_pton(AF_INET, picoConfig.hostIP, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(picoConfig.port);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        while (1)
            ; // 卡死在这
    }
    ESP_LOGI(TAG, "Socket created, sending to %s:%d", picoConfig.hostIP, picoConfig.port);
    setRgbLevel(0, 1, 0);
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        while (1)
            ; // 卡死在这
    }
    ESP_LOGI(TAG, "Successfully connected");
    setRgbLevel(1, 1, 0);

    BaseType_t xReturned = xTaskCreate(tcpClientRecvTask, "tcpClientRecvTask", 3000, NULL, tskIDLE_PRIORITY, NULL);
    assert(xReturned == pdPASS);
    ESP_LOGI(TAG, "tcpClientRecvTask create successed!");
}

/**
 * @description:  tcp client任务
 * @param {void} *pvParameters
 * @return {*}
 */
void tcpClientSend(char *tcpSendDataPtr, int tcpSendDataLength)
{
    int err = send(sock, tcpSendDataPtr, tcpSendDataLength, 0);
    if (err < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            setRgbLevel(0, 1, 1);
            shutdown(sock, 0);
            close(sock);
            tcpInit(); // 重启
        }
    }
}