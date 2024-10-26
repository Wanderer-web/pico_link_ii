/*
 * @Author: Wanderer
 * @Date: 2023-05-26 13:39:13
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-11-21 13:01:40
 * @FilePath: \pico_link_II\components\pico_components\pico_web_server.c
 * @Description:
 */

#include "pico_web_server.h"

static const char *TAG = "pico_web_server";

#define CONFIG_WEB_MOUNT_POINT "/www"

esp_err_t init_fs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_WEB_MOUNT_POINT,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}

static const char *REST_TAG = "esp-rest";
#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context
{
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html"))
    {
        type = "text/html";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".js"))
    {
        type = "application/javascript";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".css"))
    {
        type = "text/css";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".png"))
    {
        type = "image/png";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".ico"))
    {
        type = "image/x-icon";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".svg"))
    {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/')
    {
        strlcat(filepath, "/index.html", sizeof(filepath));
    }
    else
    {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1)
    {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do
    {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1)
        {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        }
        else if (read_bytes > 0)
        {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t web_send_config(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *uart_config = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "in_method", (double)(pico_config.in_method));
    cJSON_AddNumberToObject(uart_config, "uart_rate", (double)(pico_config.uart_config.uart_rate));
    cJSON_AddNumberToObject(uart_config, "uart_data_bit", (double)(pico_config.uart_config.uart_data_bit));
    cJSON_AddNumberToObject(uart_config, "uart_parity", (double)(pico_config.uart_config.uart_parity));
    cJSON_AddNumberToObject(uart_config, "uart_stop_bit", (double)(pico_config.uart_config.uart_stop_bit));
    cJSON_AddItemToObject(root, "uart_config", uart_config);
    cJSON_AddNumberToObject(root, "spi_clk_mode", (double)(pico_config.spi_clk_mode));
    cJSON_AddNumberToObject(root, "spi_bus_mode", (double)(pico_config.spi_bus_mode));
    cJSON_AddNumberToObject(root, "out_method", (double)(pico_config.out_method));
    cJSON_AddNumberToObject(root, "wifi_mode", (double)(pico_config.wifi_mode));
    cJSON_AddNumberToObject(root, "wifi_ap_max_conn", (double)(pico_config.wifi_ap_max_conn));
    cJSON_AddNumberToObject(root, "wifi_ap_channel", (double)(pico_config.wifi_ap_channel));
    cJSON_AddStringToObject(root, "STA_SSID", pico_config.STA_SSID);
    cJSON_AddStringToObject(root, "STA_PWD", pico_config.STA_PWD);
    cJSON_AddStringToObject(root, "AP_SSID", pico_config.AP_SSID);
    cJSON_AddStringToObject(root, "AP_PWD", pico_config.AP_PWD);
    cJSON_AddNumberToObject(root, "two_way_en", (double)(pico_config.two_way_en));
    cJSON_AddNumberToObject(root, "work_mode", (double)(pico_config.work_mode));
    cJSON_AddStringToObject(root, "remote_ip", pico_config.remote_ip);
    cJSON_AddNumberToObject(root, "remote_port", (double)(pico_config.remote_port));
    cJSON_AddStringToObject(root, "local_gw", pico_config.local_gw);
    cJSON_AddStringToObject(root, "local_netmask", pico_config.local_netmask);
    cJSON_AddStringToObject(root, "local_ip", pico_config.local_ip);
    cJSON_AddNumberToObject(root, "local_port", (double)(pico_config.local_port));

    esp_err_t ret;
    char *json_ptr = cJSON_Print(root);
    httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    ret = httpd_resp_send(req, json_ptr, strlen(json_ptr));
    ESP_LOGI(TAG, "send %d bytes", strlen(json_ptr));

    cJSON_free((void *)json_ptr);
    cJSON_Delete(root);

    return ret;
}

static esp_err_t config_get_handler(httpd_req_t *req)
{
    return web_send_config(req);
}

static esp_err_t config_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';
    ESP_LOGI(TAG, "recived data length is :%d", total_len);
    ESP_LOGI(TAG, "%s", buf);
    ESP_LOGI(TAG, "wifi data recived!");

    cJSON *root = cJSON_Parse(buf);
    cJSON *uart_config = cJSON_GetObjectItem(root, "uart_config");

    pico_config.in_method = (uint8_t)(cJSON_GetObjectItem(root, "in_method")->valueint);
    pico_config.uart_config.uart_rate = (uint32_t)(cJSON_GetObjectItem(uart_config, "uart_rate")->valueint);
    pico_config.uart_config.uart_data_bit = (uint8_t)(cJSON_GetObjectItem(uart_config, "uart_data_bit")->valueint);
    pico_config.uart_config.uart_parity = (uint8_t)(cJSON_GetObjectItem(uart_config, "uart_parity")->valueint);
    pico_config.uart_config.uart_stop_bit = (uint8_t)(cJSON_GetObjectItem(uart_config, "uart_stop_bit")->valueint);
    pico_config.spi_clk_mode = (uint8_t)(cJSON_GetObjectItem(root, "spi_clk_mode")->valueint);
    pico_config.spi_bus_mode = (uint8_t)(cJSON_GetObjectItem(root, "spi_bus_mode")->valueint);
    pico_config.out_method = (uint8_t)(cJSON_GetObjectItem(root, "out_method")->valueint);
    pico_config.wifi_mode = (uint8_t)(cJSON_GetObjectItem(root, "wifi_mode")->valueint);
    pico_config.wifi_ap_max_conn = (uint8_t)(cJSON_GetObjectItem(root, "wifi_ap_max_conn")->valueint);
    pico_config.wifi_ap_channel = (uint8_t)(cJSON_GetObjectItem(root, "wifi_ap_channel")->valueint);
    memset(pico_config.STA_SSID, 0, PICO_SSID_LENGTH);
    memcpy(pico_config.STA_SSID, cJSON_GetObjectItem(root, "STA_SSID")->valuestring, PICO_SSID_LENGTH);
    memset(pico_config.STA_PWD, 0, PICO_PWD_LENGTH);
    memcpy(pico_config.STA_PWD, cJSON_GetObjectItem(root, "STA_PWD")->valuestring, PICO_PWD_LENGTH);
    memset(pico_config.AP_SSID, 0, PICO_SSID_LENGTH);
    memcpy(pico_config.AP_SSID, cJSON_GetObjectItem(root, "AP_SSID")->valuestring, PICO_SSID_LENGTH);
    memset(pico_config.AP_PWD, 0, PICO_PWD_LENGTH);
    memcpy(pico_config.AP_PWD, cJSON_GetObjectItem(root, "AP_PWD")->valuestring, PICO_PWD_LENGTH);
    pico_config.two_way_en = (uint8_t)(cJSON_GetObjectItem(root, "two_way_en")->valueint);
    pico_config.work_mode = (uint8_t)(cJSON_GetObjectItem(root, "work_mode")->valueint);
    memset(pico_config.remote_ip, 0, PICO_IP_LENGTH);
    memcpy(pico_config.remote_ip, cJSON_GetObjectItem(root, "remote_ip")->valuestring, PICO_IP_LENGTH);
    pico_config.remote_port = (uint16_t)(cJSON_GetObjectItem(root, "remote_port")->valueint);
    memset(pico_config.local_gw, 0, PICO_IP_LENGTH);
    memcpy(pico_config.local_gw, cJSON_GetObjectItem(root, "local_gw")->valuestring, PICO_IP_LENGTH);
    memset(pico_config.local_netmask, 0, PICO_NETMASK_LENGTH);
    memcpy(pico_config.local_netmask, cJSON_GetObjectItem(root, "local_netmask")->valuestring, PICO_NETMASK_LENGTH);
    memset(pico_config.local_ip, 0, PICO_IP_LENGTH);
    memcpy(pico_config.local_ip, cJSON_GetObjectItem(root, "local_ip")->valuestring, PICO_IP_LENGTH);
    pico_config.local_port = (uint16_t)(cJSON_GetObjectItem(root, "local_port")->valueint);

    write_config_to_flash();

    cJSON_Delete(root);
    ESP_LOGI(TAG, "json load finished");
    httpd_resp_sendstr(req, "success");
    return ESP_OK;
}

static esp_err_t config_reset_handler(httpd_req_t *req)
{
    pico_reset_config();
    return web_send_config(req);
}

esp_err_t start_web_server(const char *base_path)
{
    ESP_ERROR_CHECK(init_fs());

    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    httpd_uri_t config_get_uri = {
        .uri = "/api/get_config",
        .method = HTTP_GET,
        .handler = config_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &config_get_uri);

    httpd_uri_t config_post_uri = {
        .uri = "/api/post_config",
        .method = HTTP_POST,
        .handler = config_post_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &config_post_uri);

    httpd_uri_t config_reset_uri = {
        .uri = "/api/reset_config",
        .method = HTTP_GET,
        .handler = config_reset_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &config_reset_uri);

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context};
    httpd_register_uri_handler(server, &common_get_uri);

    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}
