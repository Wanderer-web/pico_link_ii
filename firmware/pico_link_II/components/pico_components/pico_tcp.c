/*
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-12-27 10:31:11
 * @FilePath: \pico_link_II\components\pico_components\pico_tcp.c
 * @Description:
 */

#include "pico_tcp.h"

static const char *TAG = "pico_tcp";

static uint8_t is_tcp_init = false;

static int sock;
static int listen_sock;

static struct sockaddr_in dest_addr;

static struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
static socklen_t socklen = sizeof(source_addr);

static TaskHandle_t xHandle = NULL;

static SemaphoreHandle_t xMutex_socket_error = NULL; // 互斥量量表明SOCKET检测到错误

static WORD_ALIGNED_ATTR char *tcp_rx_buffer[PICO_TX_BUF_NUM_MAX];
static uint8_t PICO_TX_BUF_NUM = 0;
static uint8_t tcp_rx_buffer_idx = 0;
static WORD_ALIGNED_ATTR char *tcp_rx_buffer_now = NULL;

/**
 * @description: tcp client 接收任务
 * @param {void} *arg
 * @return {*}
 */
void tcp_client_recv_task(void *arg)
{
    int len = 0;
    BaseType_t ret = pdFALSE;
    while (1)
    {
        if (is_tcp_init)
        {
            len = recv(sock, tcp_rx_buffer_now, PICO_TX_BUF_SIZE, 0);
            // Error occurred during receiving or host closed connection
            if (len <= 0)
            {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                if (sock != -1)
                {
                    if (xSemaphoreTake(xMutex_socket_error, (TickType_t)0) == pdTRUE) // 上锁
                    {
                        pico_tcp_client_init();              // 重启
                        xSemaphoreGive(xMutex_socket_error); // 释放锁
                    }
                }
            }
            // Data received
            else
            {
                if (pico_config.in_method == IN_METHOD_UART)
                {
                    ret = uart_send_queue(tcp_rx_buffer_now, len);
                }
                else if (pico_config.in_method == IN_METHOD_SPI)
                {
                    ret = spi_send_queue(tcp_rx_buffer_now, len);
                }
                if (ret == pdTRUE) // 成功发送则切换到下一个接收缓存
                {
                    tcp_rx_buffer_idx = (tcp_rx_buffer_idx + 1) % PICO_TX_BUF_NUM;
                    tcp_rx_buffer_now = tcp_rx_buffer[tcp_rx_buffer_idx]; // 多段 ping-ping
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
 * @description: tcp client初始化
 * @param {*}
 * @return {*}
 */
void pico_tcp_client_init(void)
{
    do
    {
        is_tcp_init = false;
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
        inet_pton(AF_INET, pico_config.remote_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(pico_config.remote_port);
        sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            continue;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", pico_config.remote_ip, pico_config.remote_port);
        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            continue;
        }
        ESP_LOGI(TAG, "Successfully connected");

        if (pico_config.two_way_en && xHandle == NULL) // 避免重复创建接收任务
        {
            xMutex_socket_error = xSemaphoreCreateMutex();

            PICO_TX_BUF_NUM = pico_config.in_method == IN_METHOD_UART ? PICO_TX_BUF_NUM_UART : pico_config.spi_bus_mode == SPI_BUS_FD_1bit ? PICO_TX_BUF_NUM_FD_SPI
                                                                                                                                           : PICO_TX_BUF_NUM_HD_SPI;

            for (uint8_t i = 0; i < PICO_TX_BUF_NUM; i++) // 创建TCP接收缓存
            {
                tcp_rx_buffer[i] = (WORD_ALIGNED_ATTR char *)heap_caps_malloc(PICO_TX_BUF_SIZE, MALLOC_CAP_DMA); // 四字节对齐
                assert(tcp_rx_buffer[i] != NULL);
                memset(tcp_rx_buffer[i], 0, PICO_TX_BUF_SIZE);
            }
            tcp_rx_buffer_now = tcp_rx_buffer[tcp_rx_buffer_idx];

            BaseType_t xReturned = xTaskCreatePinnedToCore(tcp_client_recv_task, "tcp_client_recv_task", PICO_SLAVE_TASK_STACK_SIZE, NULL, PICO_SLAVE_TASK_PRIORITY, &xHandle, PICO_SLAVE_TASK_CORE);
            assert(xReturned == pdPASS);
            ESP_LOGI(TAG, "tcp_client_recv_task create successed!");
        }
        is_tcp_init = true;
        SET_RGB_BLUE;
        break;
    } while (1);
}

/**
 * @description: tcp client 发送数据
 * @param {char} *tcp_send_data 数据指针
 * @param {int} tcp_send_data_len 数据长度
 * @return {*}
 */
void IRAM_ATTR tcp_client_send(char *tcp_send_data, int tcp_send_data_len)
{
    if (is_tcp_init)
    {
        UPLOAD_LED_ON;
        int err = send(sock, tcp_send_data, tcp_send_data_len, 0);
        UPLOAD_LED_OFF;
        if (err < 0)
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            if (sock != -1)
            {
                if (pico_config.two_way_en)
                {
                    if (xSemaphoreTake(xMutex_socket_error, (TickType_t)0) == pdTRUE) // 上锁
                    {
                        pico_tcp_client_init();              // 重启
                        xSemaphoreGive(xMutex_socket_error); // 释放锁
                    }
                }
                else
                {
                    pico_tcp_client_init(); // 重启
                }
            }
        }
    }
}

/**
 * @description: tcp server 接收任务
 * @param {void} *arg
 * @return {*}
 */
void tcp_server_recv_task(void *arg)
{
    int len = 0;
    BaseType_t ret = pdFALSE;
    while (1)
    {
        if (is_tcp_init)
        {
            len = recv(sock, tcp_rx_buffer_now, PICO_TX_BUF_SIZE, 0);
            // Error occurred during receiving or host closed connection
            if (len <= 0)
            {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                if (sock != -1)
                {
                    if (xSemaphoreTake(xMutex_socket_error, (TickType_t)0) == pdTRUE) // 上锁
                    {
                        pico_tcp_server_init();              // 重启
                        xSemaphoreGive(xMutex_socket_error); // 释放锁
                    }
                }
            }
            // Data received
            else
            {
                if (pico_config.in_method == IN_METHOD_UART)
                {
                    ret = uart_send_queue(tcp_rx_buffer_now, len);
                }
                else if (pico_config.in_method == IN_METHOD_SPI)
                {
                    ret = spi_send_queue(tcp_rx_buffer_now, len);
                }
                if (ret == pdTRUE) // 成功发送则切换到下一个接收缓存
                {
                    tcp_rx_buffer_idx = (tcp_rx_buffer_idx + 1) % PICO_TX_BUF_NUM;
                    tcp_rx_buffer_now = tcp_rx_buffer[tcp_rx_buffer_idx]; // 多段 ping-ping
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
 * @description: tcp server初始化
 * @param {*}
 * @return {*}
 */
void pico_tcp_server_init(void)
{
    do
    {
        is_tcp_init = false;
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (pico_config.wifi_mode == PICO_WIFI_MODE_STA && !is_sta_connect)
        {
            continue;
        }
        SET_RGB_PURPLE;
        shutdown(sock, 0);
        close(sock);
        close(listen_sock);
        ESP_LOGI(TAG, "Shutting down socket and restarting...");

        int addr_family = AF_INET;
        int ip_protocol = IPPROTO_IP;
        int keepAlive = 1;
        int keepIdle = 5;
        int keepInterval = 5;
        int keepCount = 3;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(pico_config.local_port);

        listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            continue;
        }
        int opt = 1;
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        ESP_LOGI(TAG, "Socket created");

        int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            continue;
        }
        ESP_LOGI(TAG, "Socket bound, port %d", pico_config.local_port);

        err = listen(listen_sock, 1);
        if (err != 0)
        {
            ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
            continue;
        }

        ESP_LOGI(TAG, "Socket listening");

        sock = accept(listen_sock, (struct sockaddr *)&source_addr, &socklen);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            continue;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

        char addr_str[16];
        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);

        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        if (pico_config.two_way_en && xHandle == NULL) // 避免重复创建接收任务
        {
            xMutex_socket_error = xSemaphoreCreateMutex();

            PICO_TX_BUF_NUM = pico_config.in_method == IN_METHOD_UART ? PICO_TX_BUF_NUM_UART : pico_config.spi_bus_mode == SPI_BUS_FD_1bit ? PICO_TX_BUF_NUM_FD_SPI
                                                                                                                                           : PICO_TX_BUF_NUM_HD_SPI;

            for (uint8_t i = 0; i < PICO_TX_BUF_NUM; i++) // 创建TCP接收缓存
            {
                tcp_rx_buffer[i] = (WORD_ALIGNED_ATTR char *)heap_caps_malloc(PICO_TX_BUF_SIZE, MALLOC_CAP_DMA); // 四字节对齐
                assert(tcp_rx_buffer[i] != NULL);
                memset(tcp_rx_buffer[i], 0, PICO_TX_BUF_SIZE);
            }
            tcp_rx_buffer_now = tcp_rx_buffer[tcp_rx_buffer_idx];

            BaseType_t xReturned = xTaskCreatePinnedToCore(tcp_server_recv_task, "tcp_server_recv_task", PICO_SLAVE_TASK_STACK_SIZE, NULL, PICO_SLAVE_TASK_PRIORITY, &xHandle, PICO_SLAVE_TASK_CORE);
            assert(xReturned == pdPASS);
            ESP_LOGI(TAG, "tcp_server_recv_task create successed!");
        }
        is_tcp_init = true;
        SET_RGB_BLUE;
        break;
    } while (1);
}

/**
 * @description: tcp server 发送数据
 * @param {char} *tcp_send_data 数据指针
 * @param {int} tcp_send_data_len 数据长度
 * @return {*}
 */
void IRAM_ATTR tcp_server_send(char *tcp_send_data, int tcp_send_data_len)
{
    if (is_tcp_init)
    {
        UPLOAD_LED_ON;
        int err = send(sock, tcp_send_data, tcp_send_data_len, 0);
        UPLOAD_LED_OFF;
        if (err < 0)
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            if (sock != -1)
            {
                if (pico_config.two_way_en)
                {
                    if (xSemaphoreTake(xMutex_socket_error, (TickType_t)0) == pdTRUE) // 上锁
                    {
                        pico_tcp_server_init();              // 重启
                        xSemaphoreGive(xMutex_socket_error); // 释放锁
                    }
                }
                else
                {
                    pico_tcp_server_init(); // 重启
                }
            }
        }
    }
}