/*
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-03-21 20:56:36
 * @FilePath: \pico_link_II\src\main.c
 * @Description:
 */

#include "headfile.h"

static const char *TAG = "app_main";

void socketSend(char *socketSendDataPtr, int socketSendDataLength)
{
    if (picoConfig.socket == SOCKET_UDP)
    {
        udpClientSend(socketSendDataPtr, socketSendDataLength);
    }
    else if (picoConfig.socket == SOCKET_TCP)
    {
        tcpClientSend(socketSendDataPtr, socketSendDataLength);
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ledInit();
    configInit();
    ESP_LOGI(TAG, "start working");
    wifi_init_sta();
    ESP_LOGI(TAG, "socket:%d", picoConfig.socket);
    if (picoConfig.socket == SOCKET_UDP)
    {
        udpInit(); // udp client 初始化
    }
    else if (picoConfig.socket == SOCKET_TCP)
    {
        tcpInit(); // tcp client 初始化
    }
    // 根据选择协议启动相应接收任务
    ESP_LOGI(TAG, "protocol:%d", picoConfig.protocol);
    uartInit();
    if (picoConfig.protocol == PROTOCOL_UART)
    {
        int uartRecvDataLength = 0;
        char *uartRecvDataPtr = (char *)malloc(UART_RX_BUF_SIZE); // 创建串口接收缓冲区
        assert(uartRecvDataPtr != NULL);
        while (1)
        {
            uartPollRecv(uartRecvDataPtr, &uartRecvDataLength);
            if (uartRecvDataLength > 0)
            {
                setTranLedLevel(0);
                socketSend(uartRecvDataPtr, uartRecvDataLength);
                setTranLedLevel(1);
            }
        }
    }
    else if (picoConfig.protocol == PROTOCOL_SPI)
    {
        spiInit();
        int spiRecvDataLength = 0;
        WORD_ALIGNED_ATTR char *spiRecvDataPtr = (WORD_ALIGNED_ATTR char *)malloc(SPI_RX_BUF_SIZE); // 四字节对齐
        assert(spiRecvDataPtr != NULL);
        while (1)
        {
            spiPollRecv(spiRecvDataPtr, &spiRecvDataLength);
            if (spiRecvDataLength > 0)
            {
                setTranLedLevel(0);
                socketSend(spiRecvDataPtr, spiRecvDataLength);
                setTranLedLevel(1);
            }
        }
    }
}
