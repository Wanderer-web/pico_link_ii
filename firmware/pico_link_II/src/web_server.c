/*
 * @Author: Wanderer
 * @Date: 2022-04-26 20:42:37
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-03-13 20:34:49
 * @FilePath: \pico_link_II\src\web_server.c
 * @Description:
 */

#include "web_server.h"

/* Max length a file path can have on storage */
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

/* Max size of an individual file. Make sure this
 * value is same as that set in upload_script.html */
#define MAX_FILE_SIZE (200 * 1024) // 200 KB
#define MAX_FILE_SIZE_STR "200KB"

/* Scratch buffer size */
#define SCRATCH_BUFSIZE 8192

static const char *TAG = "web_server";

struct file_server_data
{
    /* Base path of file storage */
    char base_path[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
};

/* Function to initialize SPIFFS */
static esp_err_t init_spiffs(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5, // This decides the maximum number of files that can be created on the storage
        .format_if_mount_failed = true};

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
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    return ESP_OK;
}

/* Send HTTP response with a run-time generated html consisting of
 * a list of all files and folders under the requested path.
 * In case of SPIFFS this returns empty list when path is any
 * string other than '/', since SPIFFS doesn't support directories */
static esp_err_t http_resp_dir_html(httpd_req_t *req, const char *dirpath)
{
    extern const unsigned char upload_script_start[] asm("_binary_upload_script_html_start");
    extern const unsigned char upload_script_end[] asm("_binary_upload_script_html_end");
    const size_t upload_script_size = (upload_script_end - upload_script_start);

    /* Add file upload form and script which on execution sends a POST request to /upload */
    httpd_resp_send(req, (const char *)upload_script_start, upload_script_size);

    return ESP_OK;
}

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
static const char *get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest)
    {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash)
    {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize)
    {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

/* Handler to download a file kept on the server */
static esp_err_t download_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri, sizeof(filepath));
    if (!filename)
    {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* If name has trailing '/', respond with directory contents */
    if (filename[strlen(filename) - 1] == '/')
    {
        printf("return http_resp_dir_html!\r\n");
        return http_resp_dir_html(req, filepath);
    }

    return ESP_OK;
}

static esp_err_t get_info_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "get request get!");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "SSID", picoConfig.SSID);
    cJSON_AddStringToObject(root, "PWD", picoConfig.PWD);
    cJSON_AddNumberToObject(root, "protocol", (double)(picoConfig.protocol));
    cJSON_AddNumberToObject(root, "uartSpeed", (double)(picoConfig.uartSpeed));
    cJSON_AddNumberToObject(root, "socket", (double)(picoConfig.socket));
    cJSON_AddStringToObject(root, "hostIP", picoConfig.hostIP);
    cJSON_AddNumberToObject(root, "port", (double)(picoConfig.port));

    char *ptr = cJSON_Print(root);
    httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    httpd_resp_send(req, ptr, strlen(ptr));
    ESP_LOGI(TAG, "send %d bytes", strlen(ptr));

    cJSON_free((void *)ptr);
    cJSON_Delete(root);

    return ESP_OK;
}

static esp_err_t send_config_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((struct file_server_data *)(req->user_ctx))->scratch;
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
    printf("recived data length is :%d\n", total_len);
    for (int i = 0; i < total_len; i++)
    {
        putchar(buf[i]);
    }
    printf("\r\nwifi data recived!\r\n");

    cJSON *root = cJSON_Parse(buf);
    char *SSID = cJSON_GetObjectItem(root, "SSID")->valuestring;
    char *PWD = cJSON_GetObjectItem(root, "PWD")->valuestring;
    uint8_t protocol = (uint8_t)(cJSON_GetObjectItem(root, "protocol")->valueint);
    uint32_t uartSpeed = (uint32_t)(cJSON_GetObjectItem(root, "uartSpeed")->valueint);
    uint8_t socket = (uint8_t)(cJSON_GetObjectItem(root, "socket")->valueint);
    char *hostIP = cJSON_GetObjectItem(root, "hostIP")->valuestring;
    uint16_t port = (uint16_t)(cJSON_GetObjectItem(root, "port")->valueint);

    memset(picoConfig.SSID, 0, 32);
    memcpy(picoConfig.SSID, SSID, 32);
    memset(picoConfig.PWD, 0, 64);
    memcpy(picoConfig.PWD, PWD, 64);
    picoConfig.protocol = protocol;
    picoConfig.uartSpeed = uartSpeed;
    picoConfig.socket = socket;
    memset(picoConfig.hostIP, 0, 16);
    memcpy(picoConfig.hostIP, hostIP, 16);
    picoConfig.port = port;

    writeFlashConfig();

    cJSON_Delete(root);
    ESP_LOGI(TAG, "json load  finished");
    httpd_resp_sendstr(req, "success");
    return ESP_OK;
}
/* Function to start the file server */

/* Function to start the file server */
esp_err_t startWebServer(const char *base_path)
{
    ESP_ERROR_CHECK(init_spiffs());
    static struct file_server_data *server_data = NULL;

    /* Validate file storage base path */
    if (!base_path || strcmp(base_path, "/spiffs") != 0)
    {
        ESP_LOGE(TAG, "File server presently supports only '/spiffs' as base path");
        return ESP_ERR_INVALID_ARG;
    }

    if (server_data)
    {
        ESP_LOGE(TAG, "File server already started");
        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    server_data = calloc(1, sizeof(struct file_server_data));
    if (!server_data)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
        return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->base_path, base_path,
            sizeof(server_data->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server");
    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }

    httpd_uri_t file_download = {
        .uri = "/", // Match all URIs of type /path/to/file
        .method = HTTP_GET,
        .handler = download_get_handler,
        .user_ctx = server_data // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_download);

    httpd_uri_t pico_info = {
        .uri = "/info", // Match all URIs of type /path/to/file
        .method = HTTP_GET,
        .handler = get_info_handler,
        .user_ctx = server_data // Pass server data as context
    };
    httpd_register_uri_handler(server, &pico_info);

    httpd_uri_t pico_config = {
        .uri = "/config", // Match all URIs of type /upload/path/to/file
        .method = HTTP_POST,
        .handler = send_config_handler,
        .user_ctx = server_data // Pass server data as context
    };
    httpd_register_uri_handler(server, &pico_config);

    return ESP_OK;
}
