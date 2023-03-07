/*
 * @Author: Wanderer
 * @Date: 2022-04-24 20:05:36
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-03-05 15:47:59
 * @FilePath: \pico_link_II\src\spi.c
 * @Description:
 */

#include "spi.h"

#define GPIO_MOSI (7)
#define GPIO_MISO (2)
#define GPIO_SCLK (6)
#define GPIO_CS (10)

#define RCV_HOST (SPI2_HOST)

static const char *TAG = "spi_recv";

static spi_slave_transaction_t t;

/**
 * @description: SPI初始化
 * @param {*}
 * @return {*}
 */
void spiInit(void)
{
    esp_err_t ret;

    // Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI_RX_BUF_SIZE,
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .mode = 3,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0,
    };

    // Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    // Initialize SPI slave interface
    ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO); // SPI DMA传输
    assert(ret == ESP_OK);

    memset(&t, 0, sizeof(t));
    t.length = SPI_RX_BUF_SIZE * 8;
    t.tx_buffer = NULL;

    ESP_LOGI(TAG, "spi init done");
}

/**
 * @description: spi轮询接收
 * @param {char} *spiRecvDataPtr
 * @param {int} *spiRecvDataLengthPtr
 * @return {*}
 */
void spiPollRecv(char *spiRecvDataPtr, int *spiRecvDataLengthPtr)
{
    t.rx_buffer = spiRecvDataPtr;
    ESP_ERROR_CHECK(spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY));
    // spi_slave_transmit does not return until the master has done a transmission, so by here we have sent our data and
    // received data from the master
    *spiRecvDataLengthPtr = t.trans_len / 8; // 获取接收数据长度
}
