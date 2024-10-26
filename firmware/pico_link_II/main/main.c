/*
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2024-10-26 16:22:19
 * @FilePath: \pico_link_ii\firmware\pico_link_II\main\main.c
 * @Description:
 */

#include "pico_headfile.h"

static const char *TAG = "app_main";

static QueueHandle_t xQueue = NULL;

/**
 * @description: 将数据拆分成多个包发送到消息队列
 * @param {char} *data 数据头指针
 * @param {int} data_len
 * @return {*}
 */
static inline void split_data_and_send_queue(char *data, int data_len)
{
    pico_queue_message_t message;
    // uint8_t message_num = 0;
    char *tmp_ptr = data;
    while (data_len > PICO_QUEUE_PKG_LENGTH)
    {
        message.data = tmp_ptr;
        message.len = PICO_QUEUE_PKG_LENGTH;
        xQueueSend(xQueue, &message, portMAX_DELAY);
        // message_num++;
        data_len -= PICO_QUEUE_PKG_LENGTH;
        tmp_ptr += PICO_QUEUE_PKG_LENGTH;
    }
    if (data_len > 0)
    {
        message.data = tmp_ptr;
        message.len = data_len;
        xQueueSend(xQueue, &message, portMAX_DELAY);
        // message_num++;
    }
    // while (message_num < PICO_SOCKET_QUEUE_SIZE) // 用气泡把消息队列填满
    // {
    //     message_ptr.data = NULL;
    //     message.len = 0;
    //     xQueueSend(xQueue, &message, portMAX_DELAY);
    //     message_num++;
    // }
}

/**
 * @description:
 * @param {char} *socket_send_data
 * @param {int} socket_send_data_len
 * @return {*}
 */
static inline void socket_send(char *socket_send_data, int socket_send_data_len)
{
    // int64_t start_time = esp_timer_get_time();
    if (pico_config.out_method == OUT_METHOD_UDP)
    {
        if (pico_config.work_mode == WORK_MODE_CLIENT)
        {
            udp_client_send(socket_send_data, socket_send_data_len);
        }
        else if (pico_config.work_mode == WORK_MODE_SERVER)
        {
            udp_server_send(socket_send_data, socket_send_data_len);
        }
    }
    else if (pico_config.out_method == OUT_METHOD_TCP)
    {
        if (pico_config.work_mode == WORK_MODE_CLIENT)
        {
            tcp_client_send(socket_send_data, socket_send_data_len);
        }
        else if (pico_config.work_mode == WORK_MODE_SERVER)
        {
            tcp_server_send(socket_send_data, socket_send_data_len);
        }
    }
    // ESP_LOGI(TAG, "socket %d take %ld us", pico_config.out_method, (int32_t)(esp_timer_get_time() - start_time));
}

/**
 * @description: TCP/UDP发送任务
 * @param {void} *arg
 * @return {*}
 */
void socket_send_task(void *arg) // UART/SPI -> UDP/TCP
{
    pico_queue_message_t socket_send_message;
    memset(&socket_send_message, 0, sizeof(pico_queue_message_t));
    while (1)
    {
        xQueueReceive(xQueue, &socket_send_message, portMAX_DELAY);
        if (pico_config.wifi_mode != PICO_WIFI_MODE_STA || is_sta_connect)
        {
            if (socket_send_message.len > 0)
            {
                socket_send(socket_send_message.data, socket_send_message.len);
            }
        }
    }
}

void uart_recv_task(void *arg) // UART
{
    char *uart_recv_data_start = (char *)malloc(PICO_UART_RX_PKG_NUM * PICO_QUEUE_PKG_LENGTH); // 创建UART接收环形缓冲区
    assert(uart_recv_data_start != NULL);
    int uart_recv_data_idx = 0;
    char *uart_recv_data_now = uart_recv_data_start;
    pico_queue_message_t uart_recv_message;
    memset(&uart_recv_message, 0, sizeof(pico_queue_message_t));
    while (1)
    {
        uart_recv_message.len = uart_poll_recv(uart_recv_data_now); // UART有硬件FIFO, 可以一段一段发送
        if (uart_recv_message.len > 0)
        {
            uart_recv_message.data = uart_recv_data_now;
            xQueueSend(xQueue, &uart_recv_message, portMAX_DELAY);                                  // 发送一个包
            uart_recv_data_idx = (uart_recv_data_idx + 1) % PICO_UART_RX_PKG_NUM;                   // 更新缓存索引
            uart_recv_data_now = uart_recv_data_start + uart_recv_data_idx * PICO_QUEUE_PKG_LENGTH; // 环形多段ping-pong
        }
    }
}

void spi_recv_task(void *arg) // SPI
{
    int spi_recv_data_len = 0; // SPI接收的数据长度
    WORD_ALIGNED_ATTR char *spi_recv_data_now = NULL;
    while (1)
    {
        if (pico_config.spi_bus_mode == SPI_BUS_FD_1bit)
        {
            spi_recv_data_len = spi_poll_recv(&spi_recv_data_now); // SPI需要先接收一段完整的数据流, 再拆分发送
        }
        else
        {
            spi_recv_data_len = spi_hd_poll_recv(&spi_recv_data_now); // SPI需要先接收一段完整的数据流, 再拆分发送
        }
        if (spi_recv_data_len > 0)
        {
            split_data_and_send_queue(spi_recv_data_now, spi_recv_data_len); // 拆分成多个包发送
        }
    }
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    pico_leds_init();
    // pico_write_encrypt_and_check(); // 生成密钥并校验
    // pico_encrypt_check(); // 校验密钥
    pico_config_init();   // 配置参数初始化
    pico_uart_init();     // 输入方式不为UART时, UART波特率115200
    ESP_LOGI(TAG, "start working");
    // 根据协议启动相应发送任务
    ESP_LOGI(TAG, "out method: %d", pico_config.out_method);
    ESP_LOGI(TAG, "wifi mode:%d", pico_config.wifi_mode);
    if (pico_config.wifi_mode == PICO_WIFI_MODE_STA)
    {
        pico_wifi_init_sta();
    }
    else if (pico_config.wifi_mode == PICO_WIFI_MODE_AP)
    {
        pico_wifi_init_softap();
    }
    ESP_LOGI(TAG, "socket:%d-%d", pico_config.out_method, pico_config.work_mode);
    if (pico_config.out_method == OUT_METHOD_UDP)
    {
        if (pico_config.work_mode == WORK_MODE_CLIENT)
        {
            pico_udp_client_init(); // udp client 初始化
        }
        else if (pico_config.work_mode == WORK_MODE_SERVER)
        {
            pico_udp_server_init(); // udp server 初始化
        }
    }
    else if (pico_config.out_method == OUT_METHOD_TCP)
    {
        if (pico_config.work_mode == WORK_MODE_CLIENT)
        {
            pico_tcp_client_init(); // tcp client 初始化
        }
        else if (pico_config.work_mode == WORK_MODE_SERVER)
        {
            pico_tcp_server_init(); // tcp server 初始化
        }
    }
    xQueue = xQueueCreate(PICO_SOCKET_QUEUE_SIZE, sizeof(pico_queue_message_t)); // 建立SOCKET发送任务消息队列
    assert(xQueue != NULL);
    BaseType_t xReturned = xTaskCreatePinnedToCore(socket_send_task, "socket_send_task", PICO_MASTER_TASK_STACK_SIZE, NULL, PICO_MASTER_TASK_PRIORITY, NULL, PICO_MASTER_TASK_CORE);
    assert(xReturned == pdPASS);

    // 根据协议启动相应接收任务
    ESP_LOGI(TAG, "in method: %d", pico_config.in_method);
    if (pico_config.in_method == IN_METHOD_UART) // UART -> UDP/TCP
    {
        BaseType_t xReturned = xTaskCreatePinnedToCore(uart_recv_task, "uart_recv_task", PICO_SLAVE_TASK_STACK_SIZE, NULL, PICO_SLAVE_TASK_PRIORITY, NULL, PICO_SLAVE_TASK_CORE);
        assert(xReturned == pdPASS);
    }
    else if (pico_config.in_method == IN_METHOD_SPI) // SPI -> UDP/TCP
    {
        if (pico_config.spi_bus_mode == SPI_BUS_FD_1bit)
        {
            pico_spi_init(); // 初始化全双工SPI, 将全部接收事务压入SPI队列
        }
        else
        {
            pico_spi_hd_init(); // 初始化半双工SPI, 将全部接收事务压入SPI队列
        }
        BaseType_t xReturned = xTaskCreatePinnedToCore(spi_recv_task, "spi_recv_task", PICO_SLAVE_TASK_STACK_SIZE, NULL, PICO_SLAVE_TASK_PRIORITY, NULL, PICO_SLAVE_TASK_CORE);
        assert(xReturned == pdPASS);
    }
}