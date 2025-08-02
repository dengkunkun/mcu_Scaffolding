/**
 * @file uart_example.c
 * @brief UART接口使用示例
 *
 * 本示例展示了如何使用类似ESP32的UART接口在STM32上实现异步串口通信
 */

#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

// 接收任务
void uart_receive_task(void *pvParameters)
{
    uart_port_t uart_port = (uart_port_t)(uintptr_t)pvParameters;
    uint8_t rx_buffer[128];
    int received_bytes;

    printf("UART%d 接收任务启动\n", uart_port);

    while (1)
    {
        // 阻塞等待接收数据，超时时间1000ms
        received_bytes = uart_read_bytes(uart_port, rx_buffer, sizeof(rx_buffer) - 1, pdMS_TO_TICKS(1000));

        if (received_bytes > 0)
        {
            // 添加字符串结束符
            rx_buffer[received_bytes] = '\0';

            printf("UART%d 收到 %d 字节: %s\n", uart_port, received_bytes, rx_buffer);

            // 回显收到的数据
            char echo_msg[256];
            snprintf(echo_msg, sizeof(echo_msg), "Echo: %s\n", rx_buffer);
            uart_write_bytes(uart_port, echo_msg, strlen(echo_msg));
        }
        else if (received_bytes == 0)
        {
            printf("UART%d 接收超时\n", uart_port);
        }
        else
        {
            printf("UART%d 接收错误\n", uart_port);
        }
    }
}

// 发送任务
void uart_send_task(void *pvParameters)
{
    uart_port_t uart_port = (uart_port_t)(uintptr_t)pvParameters;
    char tx_buffer[128];
    int counter = 0;

    printf("UART%d 发送任务启动\n", uart_port);

    while (1)
    {
        // 每5秒发送一次心跳消息
        snprintf(tx_buffer, sizeof(tx_buffer), "Heart beat %d from UART%d\n", counter++, uart_port);

        int bytes_sent = uart_write_bytes(uart_port, tx_buffer, strlen(tx_buffer));

        if (bytes_sent > 0)
        {
            printf("UART%d 发送了 %d 字节\n", uart_port, bytes_sent);
        }
        else
        {
            printf("UART%d 发送失败\n", uart_port);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// 初始化UART和相关任务
esp_err_t uart_example_init(void)
{
    esp_err_t ret;

    // 初始化UART0，缓冲区大小为1024字节
    ret = uart_async_init(UART_NUM_0, 1024);
    if (ret != ESP_OK)
    {
        printf("UART0 初始化失败: %d\n", ret);
        return ret;
    }
    printf("UART0 初始化成功\n");

    // 创建UART0接收任务
    BaseType_t task_ret = xTaskCreate(
        uart_receive_task,             // 任务函数
        "UART0_RX",                    // 任务名称
        2048,                          // 堆栈大小
        (void *)(uintptr_t)UART_NUM_0, // 任务参数
        5,                             // 任务优先级
        NULL                           // 任务句柄
    );

    if (task_ret != pdPASS)
    {
        printf("UART0 接收任务创建失败\n");
        return ESP_ERR_NO_MEM;
    }

    // 创建UART0发送任务
    task_ret = xTaskCreate(
        uart_send_task,                // 任务函数
        "UART0_TX",                    // 任务名称
        2048,                          // 堆栈大小
        (void *)(uintptr_t)UART_NUM_0, // 任务参数
        5,                             // 任务优先级
        NULL                           // 任务句柄
    );

    if (task_ret != pdPASS)
    {
        printf("UART0 发送任务创建失败\n");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

// 串口状态监控任务
void uart_monitor_task(void *pvParameters)
{
    size_t rx_buffered, tx_free;
    (void)pvParameters; // 避免未使用参数警告

    while (1)
    {
        // 获取UART0缓冲区状态
        if (uart_get_buffered_data_len(UART_NUM_0, &rx_buffered) == ESP_OK &&
            uart_get_tx_buffer_free_size(UART_NUM_0, &tx_free) == ESP_OK)
        {

            printf("UART0 状态 - RX缓冲区: %zu 字节, TX空闲: %zu 字节\n",
                   rx_buffered, tx_free);
        }

        vTaskDelay(pdMS_TO_TICKS(10000)); // 每10秒打印一次状态
    }
}

// 高级使用示例：命令处理
typedef struct
{
    char *command;
    void (*handler)(const char *args);
} command_t;

void cmd_help(const char *args)
{
    (void)args; // 避免未使用参数警告
    const char *help_msg =
        "Available commands:\n"
        "  help - Show this help\n"
        "  status - Show UART status\n"
        "  clear - Clear UART buffers\n"
        "  flush - Flush TX buffer\n"
        "  echo <text> - Echo text back\n";
    uart_write_bytes(UART_NUM_0, help_msg, strlen(help_msg));
}

void cmd_status(const char *args)
{
    (void)args; // 避免未使用参数警告
    size_t rx_buffered, tx_free;
    char status_msg[128];

    if (uart_get_buffered_data_len(UART_NUM_0, &rx_buffered) == ESP_OK &&
        uart_get_tx_buffer_free_size(UART_NUM_0, &tx_free) == ESP_OK)
    {

        snprintf(status_msg, sizeof(status_msg),
                 "UART Status: RX=%zu bytes, TX_free=%zu bytes\n",
                 rx_buffered, tx_free);
        uart_write_bytes(UART_NUM_0, status_msg, strlen(status_msg));
    }
}

void cmd_clear(const char *args)
{
    (void)args; // 避免未使用参数警告
    esp_err_t ret = uart_clear(UART_NUM_0);
    const char *msg = (ret == ESP_OK) ? "Buffers cleared\n" : "Clear failed\n";
    uart_write_bytes(UART_NUM_0, msg, strlen(msg));
}

void cmd_flush(const char *args)
{
    (void)args; // 避免未使用参数警告
    esp_err_t ret = uart_flush(UART_NUM_0);
    const char *msg = (ret == ESP_OK) ? "TX flushed\n" : "Flush failed\n";
    uart_write_bytes(UART_NUM_0, msg, strlen(msg));
}

void cmd_echo(const char *args)
{
    char echo_msg[256];
    snprintf(echo_msg, sizeof(echo_msg), "Echo: %s\n", args ? args : "");
    uart_write_bytes(UART_NUM_0, echo_msg, strlen(echo_msg));
}

// 命令表
static const command_t commands[] = {
    {"help", cmd_help},
    {"status", cmd_status},
    {"clear", cmd_clear},
    {"flush", cmd_flush},
    {"echo", cmd_echo},
    {NULL, NULL}};

// 命令处理任务
void uart_command_task(void *pvParameters)
{
    char rx_buffer[128];
    char *cmd, *args;
    int received_bytes;
    bool command_found;
    (void)pvParameters; // 避免未使用参数警告

    printf("UART 命令处理器启动\n");
    uart_write_bytes(UART_NUM_0, "UART Command Processor Ready\nType 'help' for commands\n> ", 55);

    while (1)
    {
        // 接收命令
        received_bytes = uart_read_bytes(UART_NUM_0, rx_buffer, sizeof(rx_buffer) - 1, portMAX_DELAY);

        if (received_bytes > 0)
        {
            rx_buffer[received_bytes] = '\0';

            // 去除换行符
            char *newline = strchr(rx_buffer, '\n');
            if (newline)
                *newline = '\0';

            char *carriage = strchr(rx_buffer, '\r');
            if (carriage)
                *carriage = '\0';

            // 解析命令和参数
            cmd = strtok(rx_buffer, " ");
            args = strtok(NULL, "");

            if (cmd)
            {
                command_found = false;

                // 查找并执行命令
                for (int i = 0; commands[i].command != NULL; i++)
                {
                    if (strcmp(cmd, commands[i].command) == 0)
                    {
                        commands[i].handler(args);
                        command_found = true;
                        break;
                    }
                }

                if (!command_found)
                {
                    char error_msg[128];
                    snprintf(error_msg, sizeof(error_msg), "Unknown command: %s\n", cmd);
                    uart_write_bytes(UART_NUM_0, error_msg, strlen(error_msg));
                }
            }

            // 显示提示符
            uart_write_bytes(UART_NUM_0, "> ", 2);
        }
    }
}

// 完整的初始化函数（包含命令处理器）
esp_err_t uart_advanced_example_init(void)
{
    esp_err_t ret;

    // 初始化UART
    ret = uart_async_init(UART_NUM_0, 2048);
    if (ret != ESP_OK)
    {
        return ret;
    }

    // 创建命令处理任务
    BaseType_t task_ret = xTaskCreate(
        uart_command_task,
        "UART_CMD",
        4096,
        NULL,
        6,
        NULL);

    if (task_ret != pdPASS)
    {
        return ESP_ERR_NO_MEM;
    }

    // 创建监控任务
    task_ret = xTaskCreate(
        uart_monitor_task,
        "UART_MON",
        2048,
        NULL,
        3,
        NULL);

    if (task_ret != pdPASS)
    {
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}