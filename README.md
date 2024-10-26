# Pico Link II

Pico Link II 模块是一款高速 UART/SPI 2.4G WiFi 透传模块，该模块支持多样化的配置，具有两种输入方式（UART、SPI）和两种输出方式（UDP、TCP），可自由组合成四种工作模式

- 作者: 卜林 / QQ: 1626632460
- QQ交流群: 613773009

**硬件开源链接**：https://oshwhub.com/Wander_er/75fc1408722e442589e948862d18e044

**编译固件所需的组件**

- ESP-IDF: 5.0.4
- Vue3

**先编译 Vue 工程，再编译整个固件工程**

固件工程结构说明

|              名称               |            作用             |
| :-----------------------------: | :-------------------------: |
|   components\pico_components    |         固件各组件          |
| components\espressif__led_strip |          RGB 驱动           |
|         front\pico_web          | 网页配置界面工程，基于 Vue3 |
|           main\main.c           |           主函数            |

