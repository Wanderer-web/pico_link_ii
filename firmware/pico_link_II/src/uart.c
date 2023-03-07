/*
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-02-15 19:15:32
 * @FilePath: \pico_link_II\src\uart.c
 * @Description:
 */

#include "uart.h"

#define TXD_PIN (21)
#define RXD_PIN (20)
#define RTS_PIN (7)

static uint16_t realLen = 0;
static const char *TAG = "uart_recv";

/**
 * @description: UART模式初始化
 * @param {*}
 * @return {*}
 */
void uartRecvInit(void)
{
    const uart_config_t uart_config = {
        .baud_rate = picoConfig.uartSpeed, // 波特率
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_RTS, // 开启硬件流控
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_NUM_0, UART_RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, RTS_PIN, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "uart init done");
}

/**
 * @description: UART轮询接收
 * @param {char} *uartRecvDataPtr
 * @param {int} *uartRecvDataLengthPtr
 * @return {*}
 */
void uartPollRecv(char *uartRecvDataPtr, int *uartRecvDataLengthPtr)
{
    // Read data from the UART
    uart_read_bytes(UART_NUM_0, &realLen, 2, portMAX_DELAY);
    if ((uint32_t)realLen <= UART_RX_BUF_SIZE)
    {
        *uartRecvDataLengthPtr = uart_read_bytes(UART_NUM_0, uartRecvDataPtr, (uint32_t)realLen, 20 / portTICK_PERIOD_MS); // 串口轮询最长等待20ms，低于这个值系统可能出现错误
    }
}
