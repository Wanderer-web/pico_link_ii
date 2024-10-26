/*
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-12-27 10:30:53
 * @FilePath: \pico_link_II\components\pico_components\pico_udp.c
 * @Description:
 */

#include "pico_udp.h"

static const char *TAG = "pico_udp";

static uint8_t is_udp_init = false;

static int sock;

static struct sockaddr_in dest_addr;

static struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
static socklen_t socklen = sizeof(source_addr);

static TaskHandle_t xHandle = NULL;

static SemaphoreHandle_t xMutex_socket_error = NULL; // 互斥量量表明SOCKET检测到错误

static WORD_ALIGNED_ATTR char *udp_rx_buffer[PICO_TX_BUF_NUM_MAX];
static uint8_t PICO_TX_BUF_NUM = 0;
static uint8_t udp_rx_buffer_idx = 0;
static WORD_ALIGNED_ATTR char *udp_rx_buffer_now = NULL;

/**
 * @description: udp client 接收任务
 * @param {void} *arg
 * @return {*}
 */
void udp_client_recv_task(void *arg)
{
    int len = 0;
    BaseType_t ret = pdFALSE;
    while (1)
    {
        if (is_udp_init)
        {
            len = recvfrom(sock, udp_rx_buffer_now, PICO_TX_BUF_SIZE, 0, (struct sockaddr *)&source_addr, &socklen);
            // Error occurred during receiving
            if (len <= 0)
            {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                if (sock != -1)
                {
                    if (xSemaphoreTake(xMutex_socket_error, (TickType_t)0) == pdTRUE) // 上锁
                    {
                        pico_udp_client_init();              // 重启
                        xSemaphoreGive(xMutex_socket_error); // 释放锁
                    }
                }
            }
            // Data received
            else
            {
                if (pico_config.in_method == IN_METHOD_UART)
                {
                    ret = uart_send_queue(udp_rx_buffer_now, len);
                }
                else if (pico_config.in_method == IN_METHOD_SPI)
                {
                    ret = spi_send_queue(udp_rx_buffer_now, len);
                }
                if (ret == pdTRUE) // 成功发送则切换到下一个接收缓存
                {
                    udp_rx_buffer_idx = (udp_rx_buffer_idx + 1) % PICO_TX_BUF_NUM;
                    udp_rx_buffer_now = udp_rx_buffer[udp_rx_buffer_idx]; // 多段 ping-ping
                }
                // vTaskDelay(20 / portTICK_PERIOD_MS);
            }
        }
        else
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}

/**
 * @description: udp client初始化
 * @param {*}
 * @return {*}
 */
void pico_udp_client_init(void)
{
    do
    {
        is_udp_init = false;
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (pico_config.wifi_mode == PICO_WIFI_MODE_STA && !is_sta_connect)
        {
            continue;
        }
        SET_RGB_PURPLE;
        shutdown(sock, 0);
        close(sock);
        ESP_LOGI(TAG, "Shutting down socket and restarting...");

        int addr_family = AF_INET;
        int ip_protocol = IPPROTO_IP;
        dest_addr.sin_addr.s_addr = inet_addr(pico_config.remote_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(pico_config.remote_port);
        sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            continue;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", pico_config.remote_ip, pico_config.remote_port);
        if (pico_config.two_way_en && xHandle == NULL) // 避免重复创建接收任务
        {
            xMutex_socket_error = xSemaphoreCreateMutex();

            PICO_TX_BUF_NUM = pico_config.in_method == IN_METHOD_UART ? PICO_TX_BUF_NUM_UART : pico_config.spi_bus_mode == SPI_BUS_FD_1bit ? PICO_TX_BUF_NUM_FD_SPI
                                                                                                                                           : PICO_TX_BUF_NUM_HD_SPI;

            for (uint8_t i = 0; i < PICO_TX_BUF_NUM; i++) // 创建TCP接收缓存
            {
                udp_rx_buffer[i] = (WORD_ALIGNED_ATTR char *)heap_caps_malloc(PICO_TX_BUF_SIZE, MALLOC_CAP_DMA); // 四字节对齐
                assert(udp_rx_buffer[i] != NULL);
                memset(udp_rx_buffer[i], 0, PICO_TX_BUF_SIZE);
            }
            udp_rx_buffer_now = udp_rx_buffer[udp_rx_buffer_idx];

            BaseType_t xReturned = xTaskCreatePinnedToCore(udp_client_recv_task, "udp_client_recv_task", PICO_SLAVE_TASK_STACK_SIZE, NULL, PICO_SLAVE_TASK_PRIORITY, &xHandle, PICO_SLAVE_TASK_CORE);
            assert(xReturned == pdPASS);
            ESP_LOGI(TAG, "udp_client_recv_task create successed!");
        }
        is_udp_init = true;
        SET_RGB_BLUE;
        break;
    } while (1);
}

/**
 * @description: udp client 发送数据
 * @param {char} *udp_send_data 数据指针
 * @param {int} udp_send_data_len 数据长度
 * @return {*}
 */
void IRAM_ATTR udp_client_send(char *udp_send_data, int udp_send_data_len)
{
    if (is_udp_init)
    {
        UPLOAD_LED_ON;
        int err = sendto(sock, udp_send_data, udp_send_data_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        UPLOAD_LED_OFF;
        if (err < 0 && errno != ENOMEM) // WiFi缓存不够就把这帧丢掉，不重启udp了
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            if (sock != -1)
            {
                if (pico_config.two_way_en)
                {
                    if (xSemaphoreTake(xMutex_socket_error, (TickType_t)0) == pdTRUE) // 上锁
                    {
                        pico_udp_client_init();              // 重启
                        xSemaphoreGive(xMutex_socket_error); // 释放锁
                    }
                }
                else
                {
                    pico_udp_client_init(); // 重启
                }
            }
        }
    }
}

/**
 * @description: udp server 接收任务
 * @param {void} *arg
 * @return {*}
 */
void udp_server_recv_task(void *arg)
{
    int len = 0;
    BaseType_t ret = pdFALSE;
    while (1)
    {
        if (is_udp_init)
        {
            len = recvfrom(sock, udp_rx_buffer_now, PICO_TX_BUF_SIZE, 0, (struct sockaddr *)&source_addr, &socklen);
            // Error occurred during receiving
            if (len <= 0)
            {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                if (sock != -1)
                {
                    if (xSemaphoreTake(xMutex_socket_error, (TickType_t)0) == pdTRUE) // 上锁
                    {
                        pico_udp_server_init();              // 重启
                        xSemaphoreGive(xMutex_socket_error); // 释放锁
                    }
                }
            }
            // Data received
            else
            {
                if (pico_config.in_method == IN_METHOD_UART)
                {
                    ret = uart_send_queue(udp_rx_buffer_now, len);
                }
                else if (pico_config.in_method == IN_METHOD_SPI)
                {
                    ret = spi_send_queue(udp_rx_buffer_now, len);
                }
                if (ret == pdTRUE) // 成功发送则切换到下一个接收缓存
                {
                    udp_rx_buffer_idx = (udp_rx_buffer_idx + 1) % PICO_TX_BUF_NUM;
                    udp_rx_buffer_now = udp_rx_buffer[udp_rx_buffer_idx]; // 多段 ping-ping
                }
                // vTaskDelay(20 / portTICK_PERIOD_MS);
            }
        }
        else
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}

/**
 * @description: udp server初始化
 * @return {*}
 */
void pico_udp_server_init(void)
{
    do
    {
        is_udp_init = false;
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (pico_config.wifi_mode == PICO_WIFI_MODE_STA && !is_sta_connect)
        {
            continue;
        }
        SET_RGB_PURPLE;
        shutdown(sock, 0);
        close(sock);
        ESP_LOGI(TAG, "Shutting down socket and restarting...");

        int ip_protocol = IPPROTO_IP;
        int addr_family = AF_INET;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(pico_config.local_port);
        sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            continue;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0)
        {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            continue;
        }
        ESP_LOGI(TAG, "Socket bound, port %d", pico_config.local_port);

        char addr_str[16];
        char test_rx_buffer[16];

        int len = recvfrom(sock, test_rx_buffer, sizeof(test_rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen); // 监听一次，获取目标地址

        if (len < 0)
        {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            continue;
        }
        else
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
            test_rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
            ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
            ESP_LOGI(TAG, "%s", test_rx_buffer);
            if (pico_config.two_way_en && xHandle == NULL) // 避免重复创建接收任务
            {
                xMutex_socket_error = xSemaphoreCreateMutex();

                PICO_TX_BUF_NUM = pico_config.in_method == IN_METHOD_UART ? PICO_TX_BUF_NUM_UART : pico_config.spi_bus_mode == SPI_BUS_FD_1bit ? PICO_TX_BUF_NUM_FD_SPI
                                                                                                                                               : PICO_TX_BUF_NUM_HD_SPI;

                for (uint8_t i = 0; i < PICO_TX_BUF_NUM; i++) // 创建TCP接收缓存
                {
                    udp_rx_buffer[i] = (WORD_ALIGNED_ATTR char *)heap_caps_malloc(PICO_TX_BUF_SIZE, MALLOC_CAP_DMA); // 四字节对齐
                    assert(udp_rx_buffer[i] != NULL);
                    memset(udp_rx_buffer[i], 0, PICO_TX_BUF_SIZE);
                }
                udp_rx_buffer_now = udp_rx_buffer[udp_rx_buffer_idx];

                BaseType_t xReturned = xTaskCreatePinnedToCore(udp_server_recv_task, "udp_server_recv_task", PICO_SLAVE_TASK_STACK_SIZE, NULL, PICO_SLAVE_TASK_PRIORITY, &xHandle, PICO_SLAVE_TASK_CORE);
                assert(xReturned == pdPASS);
                ESP_LOGI(TAG, "udp_server_recv_task create successed!");
            }
            is_udp_init = true;
            SET_RGB_BLUE;
        }
        break;
    } while (1);
}

/**
 * @description: udp server 发送数据
 * @param {char} *udp_send_data 数据指针
 * @param {int} udp_send_data_len 数据长度
 * @return {*}
 */
void IRAM_ATTR udp_server_send(char *udp_send_data, int udp_send_data_len)
{
    if (is_udp_init)
    {
        UPLOAD_LED_ON;
        int err = sendto(sock, udp_send_data, udp_send_data_len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
        UPLOAD_LED_OFF;
        if (err < 0 && errno != ENOMEM) // WiFi缓存不够就把这帧丢掉，不重启udp了
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            if (sock != -1)
            {
                if (pico_config.two_way_en)
                {
                    if (xSemaphoreTake(xMutex_socket_error, (TickType_t)0) == pdTRUE) // 上锁
                    {
                        pico_udp_server_init();              // 重启
                        xSemaphoreGive(xMutex_socket_error); // 释放锁
                    }
                }
                else
                {
                    pico_udp_server_init(); // 重启
                }
            }
        }
    }
}