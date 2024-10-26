/*
 * @Author: Wanderer
 * @Date: 2023-05-12 21:39:23
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-12-27 10:29:34
 * @FilePath: \pico_link_II\components\pico_components\pico_spi.c
 * @Description:
 */

#include "pico_spi.h"

#define GPIO_MOSI (7)
#define GPIO_MISO (2)
#define GPIO_SCLK (6)
#define GPIO_CS (10)

#define PICO_SPI_HOST (SPI2_HOST)

#define SPI_RX_BUF_SIZE (PICO_SPI_RX_PKG_NUM * PICO_QUEUE_PKG_LENGTH)

static const char *TAG = "pico_spi";

static WORD_ALIGNED_ATTR char *spi_recv_data[PICO_SPI_QUEUE_SIZE]; // SPI接收缓存(全双工/半双工)
static WORD_ALIGNED_ATTR char *spi_tx_buffer_now = NULL;

static spi_slave_transaction_t t[PICO_SPI_QUEUE_SIZE]; // 全双工事务描述符
static spi_slave_transaction_t *ret_trans = NULL;      // 全双工事务返回描述符

static SemaphoreHandle_t xSemaphore_tx_buffer_fresh = NULL; // 信号量表明全双工发送缓存是否更新
static SemaphoreHandle_t xSemaphore_tx_buffer_dirty = NULL; // 信号量表明全双工发送缓存是否送入传输队列

static QueueHandle_t xQueue = NULL;

/**
 * @description: spi全双工轮询接收
 * @param {char} **spi_recv_data_ptr 返回的接收缓存指针的指针
 * @return {*}
 */
int IRAM_ATTR spi_poll_recv(char **spi_recv_data_ptr)
{
    ESP_ERROR_CHECK(spi_slave_get_trans_result(PICO_SPI_HOST, &ret_trans, portMAX_DELAY)); // 获取传输事务结果
    *spi_recv_data_ptr = (char *)(ret_trans->rx_buffer);                                   // 返回接收缓存地址
    int recv_len = ret_trans->trans_len / 8;                                               // 获取接收数据长度
    // ESP_LOGI(TAG, "spi get %d B", recv_len);
    if (pico_config.two_way_en) // SPI双向通信
    {
        if (xSemaphoreTake(xSemaphore_tx_buffer_fresh, (TickType_t)10) == pdTRUE) // 新发送数据已加载到spi_tx_buffer_now
        {
            ret_trans->tx_buffer = spi_tx_buffer_now;   // 将发送数据融入下一次接收事务中
            xSemaphoreGive(xSemaphore_tx_buffer_dirty); // 通知可加载新发送数据到spi_tx_buffer_now
        }
        else
        {
            ret_trans->tx_buffer = NULL;
        }
    }
    ESP_ERROR_CHECK(spi_slave_queue_trans(PICO_SPI_HOST, ret_trans, portMAX_DELAY)); // 重新送入队列
    return recv_len;
}

/**
 * @description: SPI全双工发送任务
 * @param {void} *arg
 * @return {*}
 */
void spi_send_task(void *arg)
{
    pico_queue_message_t spi_send_message;
    memset(&spi_send_message, 0, sizeof(pico_queue_message_t));
    while (1)
    {
        xQueueReceive(xQueue, &spi_send_message, portMAX_DELAY);
        if (spi_send_message.len > 0)
        {
            DOWNLOAD_LED_ON;
            if (xSemaphoreTake(xSemaphore_tx_buffer_dirty, portMAX_DELAY)) // 等待前一次发送完成
            {
                spi_tx_buffer_now = spi_send_message.data;
                // if (len < PICO_TX_BUF_SIZE)
                // {
                //     memset((spi_tx_buffer_now + len), 0, (PICO_TX_BUF_SIZE - len));
                // }
                xSemaphoreGive(xSemaphore_tx_buffer_fresh); // 通知发送缓存已更新
            }
            DOWNLOAD_LED_OFF;
        }
    }
}

/**
 * @description: spi全双工初始化, 将传输事务送入队列
 * @return {*}
 */
void pico_spi_init(void)
{
    // Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = (pico_config.two_way_en ? PICO_TX_BUF_SIZE : SPI_RX_BUF_SIZE), // 对于全双工SPI, 最大接收字节数应和最大发送字节数一致
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .mode = pico_config.spi_clk_mode,
        .spics_io_num = GPIO_CS,
        .queue_size = PICO_SPI_QUEUE_SIZE,
        .flags = 0,
    };

    // Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    // Initialize SPI slave interface
    ESP_ERROR_CHECK(spi_slave_initialize(PICO_SPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO)); // SPI DMA传输

    for (uint8_t i = 0; i < PICO_SPI_QUEUE_SIZE; i++) // 创建SPI接收缓存并映射到传输事务
    {
        if (pico_config.two_way_en) // 对于全双工SPI, 最大接收字节数应和最大发送字节数一致
        {
            spi_recv_data[i] = (WORD_ALIGNED_ATTR char *)heap_caps_malloc((PICO_TX_BUF_SIZE), MALLOC_CAP_DMA); // 四字节对齐
            assert(spi_recv_data[i] != NULL);

            memset(&t[i], 0, sizeof(t[i]));
            t[i].length = PICO_TX_BUF_SIZE * 8;
            t[i].tx_buffer = NULL;
            t[i].rx_buffer = spi_recv_data[i];
            ESP_ERROR_CHECK(spi_slave_queue_trans(PICO_SPI_HOST, &t[i], portMAX_DELAY));
        }
        else
        {
            spi_recv_data[i] = (WORD_ALIGNED_ATTR char *)heap_caps_malloc((PICO_SPI_RX_PKG_NUM * PICO_QUEUE_PKG_LENGTH), MALLOC_CAP_DMA); // 四字节对齐
            assert(spi_recv_data[i] != NULL);

            memset(&t[i], 0, sizeof(t[i]));
            t[i].length = SPI_RX_BUF_SIZE * 8;
            t[i].tx_buffer = NULL;
            t[i].rx_buffer = spi_recv_data[i];
            ESP_ERROR_CHECK(spi_slave_queue_trans(PICO_SPI_HOST, &t[i], portMAX_DELAY));
        }
    }

    if (pico_config.two_way_en)
    {
        // 创建信号量
        xSemaphore_tx_buffer_fresh = xSemaphoreCreateBinary();
        xSemaphore_tx_buffer_dirty = xSemaphoreCreateBinary();
        xSemaphoreGive(xSemaphore_tx_buffer_dirty); // 通知发送缓存可更新

        xQueue = xQueueCreate(PICO_TX_QUEUE_SIZE, sizeof(pico_queue_message_t)); // 建立SPI发送任务消息队列
        assert(xQueue != NULL);
        BaseType_t xReturned = xTaskCreatePinnedToCore(spi_send_task, "spi_send_task", PICO_MASTER_TASK_STACK_SIZE, NULL, PICO_MASTER_TASK_PRIORITY, NULL, PICO_MASTER_TASK_CORE);
        assert(xReturned == pdPASS);
        ESP_LOGI(TAG, "spi_send_task create successed!");
    }

    ESP_LOGI(TAG, "spi init done");
}

static spi_slave_hd_data_t hd_t_rx[PICO_SPI_QUEUE_SIZE]; // 半双工接收事务描述符
static spi_slave_hd_data_t *hd_ret_trans_rx = NULL;      // 半双工接收事务返回描述符

static spi_slave_hd_data_t hd_t_tx[PICO_SPI_QUEUE_SIZE]; // 半双工发送事务描述符
static spi_slave_hd_data_t *hd_ret_trans_tx = NULL;      // 半双工发送事务返回描述符

static uint8_t hd_t_tx_in_queue_num = 0; // 在半双工SPI发送队列里的事务数量

/**
 * @description: spi半双工轮询接收
 * @param {char} **spi_recv_data_ptr 返回的接收缓存指针的指针
 * @return {*}
 */
int IRAM_ATTR spi_hd_poll_recv(char **spi_recv_data_ptr)
{
    ESP_ERROR_CHECK(spi_slave_hd_get_trans_res(PICO_SPI_HOST, SPI_SLAVE_CHAN_RX, &hd_ret_trans_rx, portMAX_DELAY)); // 获取传输事务结果
    *spi_recv_data_ptr = (char *)(hd_ret_trans_rx->data);                                                           // 返回接收缓存地址
    int recv_len = hd_ret_trans_rx->trans_len;                                                                      // 获取接收数据长度
    // ESP_LOGI(TAG, "spi get %d B", recv_len);
    ESP_ERROR_CHECK(spi_slave_hd_queue_trans(PICO_SPI_HOST, SPI_SLAVE_CHAN_RX, hd_ret_trans_rx, portMAX_DELAY)); // 重新将半双工接收事务压入队列
    return recv_len;
}

typedef struct
{
    uint8_t rd_ptr;
    uint8_t wr_ptr;
    size_t len[PICO_SPI_QUEUE_SIZE];
} hd_tx_len_fifo_t; // 存储半双工SPI发送长度的FIFO

typedef struct
{
    uint8_t magic;
    uint8_t sequence;
    uint16_t length;
} __attribute__((packed)) hd_rd_status_t; // 存储半双工SPI读状态寄存器内容

#define SPI_HD_RD_STATUS_REG (0x04) // 读状态寄存器地址

/**
 * @description: SPI半双工发送任务
 * @param {void} *arg
 * @return {*}
 */
void spi_hd_send_task(void *arg)
{
    pico_queue_message_t spi_send_message;
    memset(&spi_send_message, 0, sizeof(pico_queue_message_t));
    hd_tx_len_fifo_t hd_tx_len_fifo = {
        .rd_ptr = 0,
        .wr_ptr = 0,
    };
    hd_rd_status_t hd_rd_status = {
        .magic = 0x00,
        .sequence = 0xff,
        .length = 0,
    };
    spi_slave_hd_write_buffer(PICO_SPI_HOST, SPI_HD_RD_STATUS_REG, (uint8_t *)(&hd_rd_status), sizeof(hd_rd_status_t)); // 更新读状态寄存器
    while (1)
    {
        if (spi_slave_hd_get_trans_res(PICO_SPI_HOST, SPI_SLAVE_CHAN_TX, &hd_ret_trans_tx, 20 / portTICK_PERIOD_MS) == ESP_OK) // 半双工SPI需要经常检查向主机方发送的数据接收情况
        {
            hd_t_tx_in_queue_num--;
            hd_tx_len_fifo.rd_ptr = (hd_tx_len_fifo.rd_ptr + 1) % PICO_SPI_QUEUE_SIZE; // 读指针移动
            if (hd_t_tx_in_queue_num > 0)
            {
                hd_rd_status.magic = 0x01; // 可读
                hd_rd_status.sequence = hd_tx_len_fifo.rd_ptr;
                hd_rd_status.length = (uint16_t)(hd_tx_len_fifo.len[hd_tx_len_fifo.rd_ptr]);                                        // 读出FIFO顶元素
                spi_slave_hd_write_buffer(PICO_SPI_HOST, SPI_HD_RD_STATUS_REG, (uint8_t *)(&hd_rd_status), sizeof(hd_rd_status_t)); // 更新读状态寄存器
            }
            else
            {
                hd_rd_status.magic = 0x00; // 不可读
                hd_rd_status.sequence = 0xff;
                hd_rd_status.length = 0;
                spi_slave_hd_write_buffer(PICO_SPI_HOST, SPI_HD_RD_STATUS_REG, (uint8_t *)(&hd_rd_status), sizeof(hd_rd_status_t)); // 更新读状态寄存器
            }
        }
        if (hd_t_tx_in_queue_num < PICO_SPI_QUEUE_SIZE) // 发送事务队列没满就接收消息
        {
            if (xQueueReceive(xQueue, &spi_send_message, 20 / portTICK_PERIOD_MS) == pdTRUE && spi_send_message.len > 0)
            {
                DOWNLOAD_LED_ON;
                hd_t_tx[hd_tx_len_fifo.wr_ptr].data = (uint8_t *)(spi_send_message.data);
                hd_t_tx[hd_tx_len_fifo.wr_ptr].len = spi_send_message.len;
                spi_slave_hd_queue_trans(PICO_SPI_HOST, SPI_SLAVE_CHAN_TX, &hd_t_tx[hd_tx_len_fifo.wr_ptr], portMAX_DELAY);
                hd_t_tx_in_queue_num++;
                DOWNLOAD_LED_OFF;

                hd_tx_len_fifo.len[hd_tx_len_fifo.wr_ptr] = spi_send_message.len;          // 将传输长度写入fifo
                hd_tx_len_fifo.wr_ptr = (hd_tx_len_fifo.wr_ptr + 1) % PICO_SPI_QUEUE_SIZE; // 写指针移动
                if (hd_t_tx_in_queue_num == 1)                                             // 可读
                {
                    hd_rd_status.magic = 0x01;
                    hd_rd_status.sequence = hd_tx_len_fifo.rd_ptr;
                    hd_rd_status.length = (uint16_t)(spi_send_message.len);
                    spi_slave_hd_write_buffer(PICO_SPI_HOST, SPI_HD_RD_STATUS_REG, (uint8_t *)(&hd_rd_status), sizeof(hd_rd_status_t)); // 更新读状态寄存器
                }
            }
        }
    }
}

/**
 * @description: spi半双工初始化, 将半双工接收事务送入队列
 * @return {*}
 */
void pico_spi_hd_init(void)
{
    // Configuration for the SPI bus
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI_RX_BUF_SIZE,
        .flags = (SPICOMMON_BUSFLAG_DUAL | SPICOMMON_BUSFLAG_SLAVE),
    };

    // Configuration for the SPI slave interface
    spi_slave_hd_slot_config_t slave_hd_cfg = {
        .mode = pico_config.spi_clk_mode,
        .spics_io_num = GPIO_CS,
        .queue_size = PICO_SPI_QUEUE_SIZE,
        .flags = 0,
        .command_bits = 8,
        .address_bits = 8,
        .dummy_bits = 8,
        .dma_chan = SPI_DMA_CH_AUTO,
    };

    ESP_ERROR_CHECK(spi_slave_hd_init(PICO_SPI_HOST, &bus_cfg, &slave_hd_cfg));

    for (uint8_t i = 0; i < PICO_SPI_QUEUE_SIZE; i++) // 创建SPI接收缓存并映射到半双工接收事务
    {
        spi_recv_data[i] = (WORD_ALIGNED_ATTR char *)heap_caps_malloc((PICO_SPI_RX_PKG_NUM * PICO_QUEUE_PKG_LENGTH), MALLOC_CAP_DMA); // 四字节对齐
        assert(spi_recv_data[i] != NULL);

        memset(&hd_t_rx[i], 0, sizeof(hd_t_rx[i]));
        hd_t_rx[i].len = SPI_RX_BUF_SIZE;
        hd_t_rx[i].data = (uint8_t *)(spi_recv_data[i]);
        ESP_ERROR_CHECK(spi_slave_hd_queue_trans(PICO_SPI_HOST, SPI_SLAVE_CHAN_RX, &hd_t_rx[i], portMAX_DELAY));
    }

    if (pico_config.two_way_en)
    {
        xQueue = xQueueCreate(PICO_TX_QUEUE_SIZE, sizeof(pico_queue_message_t)); // 建立SPI发送任务消息队列
        assert(xQueue != NULL);
        BaseType_t xReturned = xTaskCreatePinnedToCore(spi_hd_send_task, "spi_hd_send_task", PICO_MASTER_TASK_STACK_SIZE, NULL, PICO_MASTER_TASK_PRIORITY, NULL, PICO_MASTER_TASK_CORE);
        assert(xReturned == pdPASS);
        ESP_LOGI(TAG, "spi_hd_send_task create successed!");
    }

    ESP_LOGI(TAG, "spi hd init done");
}

/**
 * @description: SPI发送
 * @param {char} *spi_send_data 数据头指针
 * @param {int} len 数据长度
 * @return {*}
 */
BaseType_t spi_send_queue(char *spi_send_data, int len)
{
    if (pico_config.spi_bus_mode == SPI_BUS_HD_1_2bit && (hd_t_tx_in_queue_num == PICO_SPI_QUEUE_SIZE)) // 如果半双工发送事务队列满了就不要发送消息了
    {
        return pdFALSE;
    }
    pico_queue_message_t spi_send_message = {
        .data = spi_send_data,
        .len = len,
    };
    return xQueueSend(xQueue, &spi_send_message, 20 / portTICK_PERIOD_MS); // 发送队列满了就把这帧数据丢掉不影响后续数据接收
}