/*
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-12-27 10:29:56
 * @FilePath: \pico_link_II\components\pico_components\pico_uart.c
 * @Description:
 */

#include "pico_uart.h"

#define TXD_PIN (21)
#define RXD_PIN (20)

#define UART_RX_BUF_SIZE (PICO_QUEUE_PKG_LENGTH)

static const char *TAG = "pico_uart";

static const uint8_t uart_data_bit_table[4] = {
    UART_DATA_5_BITS,
    UART_DATA_6_BITS,
    UART_DATA_7_BITS,
    UART_DATA_8_BITS,
};
static const uint8_t uart_parity_table[3] = {
    UART_PARITY_DISABLE,
    UART_PARITY_EVEN,
    UART_PARITY_ODD,
};
static const uint8_t uart_stop_bit_table[3] = {
    UART_STOP_BITS_1,
    UART_STOP_BITS_1_5,
    UART_STOP_BITS_2,
};

static QueueHandle_t xQueue = NULL;

/**
 * @description: UART轮询接收
 * @param {char} *uart_recv_data
 * @return {*}
 */
int IRAM_ATTR uart_poll_recv(char *uart_recv_data)
{
    // Read data from the UART
    return uart_read_bytes(UART_NUM_0, uart_recv_data, UART_RX_BUF_SIZE, 20 / portTICK_PERIOD_MS); // 串口轮询最长等待20ms，低于这个值系统可能出现错误
}

/**
 * @description: UART轮询接收
 * @param {char} *uart_recv_data
 * @return {*}
 */
int uart_poll_recv_256B(char *uart_recv_data)
{
    // Read data from the UART
    return uart_read_bytes(UART_NUM_0, uart_recv_data, 256, 20 / portTICK_PERIOD_MS); // 串口轮询最长等待20ms，低于这个值系统可能出现错误
}

/**
 * @description: UART发送
 * @param {char} *uart_send_data 数据头指针
 * @param {int} len 数据长度
 * @return {*}
 */
BaseType_t uart_send_queue(char *uart_send_data, int len)
{
    pico_queue_message_t uart_send_message = {
        .data = uart_send_data,
        .len = len,
    };
    return xQueueSend(xQueue, &uart_send_message, 20 / portTICK_PERIOD_MS); // 发送队列满了就把这帧数据丢掉不影响后续数据接收
}

/**
 * @description: UART发送任务
 * @param {void} *arg
 * @return {*}
 */
void uart_send_task(void *arg)
{
    pico_queue_message_t uart_send_message;
    memset(&uart_send_message, 0, sizeof(pico_queue_message_t));
    while (1)
    {
        xQueueReceive(xQueue, &uart_send_message, portMAX_DELAY);
        if (uart_send_message.len > 0)
        {
            DOWNLOAD_LED_ON;
            uart_write_bytes(UART_NUM_0, uart_send_message.data, uart_send_message.len);
            DOWNLOAD_LED_OFF;
        }
    }
}

/**
 * @description: UART模式初始化
 * @param {*}
 * @return {*}
 */
void pico_uart_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = ((pico_config.in_method == IN_METHOD_UART) ? pico_config.uart_config.uart_rate : 115200), // 波特率
        .data_bits = uart_data_bit_table[pico_config.uart_config.uart_data_bit],
        .parity = uart_parity_table[pico_config.uart_config.uart_parity],
        .stop_bits = uart_stop_bit_table[pico_config.uart_config.uart_stop_bit],
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(UART_NUM_0, ((pico_config.in_method == IN_METHOD_UART) ? (UART_RX_BUF_SIZE + 128) : 256), ((pico_config.two_way_en && pico_config.in_method == IN_METHOD_UART) ? (PICO_TX_BUF_SIZE + 128) : 0), 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    if (pico_config.two_way_en && pico_config.in_method == IN_METHOD_UART) // 创建双向传输串口发送任务
    {
        xQueue = xQueueCreate(PICO_TX_QUEUE_SIZE, sizeof(pico_queue_message_t)); // 建立UART发送任务消息队列
        assert(xQueue != NULL);
        BaseType_t xReturned = xTaskCreatePinnedToCore(uart_send_task, "uart_send_task", PICO_MASTER_TASK_STACK_SIZE, NULL, PICO_MASTER_TASK_PRIORITY, NULL, PICO_MASTER_TASK_CORE);
        assert(xReturned == pdPASS);
        ESP_LOGI(TAG, "uart_send_task create successed!");
    }

    ESP_LOGI(TAG, "uart init done");
}
