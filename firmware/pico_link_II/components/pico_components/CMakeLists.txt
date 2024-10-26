idf_component_register(SRCS "pico_encrypt.c" 
                            "pico_uart.c" 
                            "pico_config.c"
                            "pico_flash.c"
                            "pico_leds.c"
                            "pico_spi.c"
                            "pico_tcp.c"
                            "pico_udp.c"
                            "pico_web_server.c"
                            "pico_wifi_ap.c"
                            "pico_wifi_sta.c"
                        REQUIRES driver 
                                 efuse 
                                 esp_timer 
                                 lwip 
                                 vfs 
                                 esp_wifi 
                                 esp_event 
                                 esp_netif 
                                 esp_eth 
                                 esp_phy
                                 nvs_flash
                                 esp_http_server
                                 spiffs
                                 json
                                 sdmmc
                                 fatfs
                                 led_strip
                    INCLUDE_DIRS "include")

set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../front/pico_web")
spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
