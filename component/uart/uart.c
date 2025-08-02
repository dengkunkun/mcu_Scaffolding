#include "hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "semphr.h"

#include "uart.h"
#include "worker.h"
#include <string.h>

// UART设备结构体
typedef struct
{
    UART_HandleTypeDef *hal_uart;   // HAL UART句柄
    StreamBufferHandle_t tx_stream; // 发送流缓冲区
    StreamBufferHandle_t rx_stream; // 接收流缓冲区
    SemaphoreHandle_t tx_mutex;     // 发送互斥锁
    SemaphoreHandle_t rx_mutex;     // 接收互斥锁
    bool initialized;               // 初始化标志
    bool tx_busy;                   // 发送忙标志
    uint32_t tx_buffer_size;        // 发送缓冲区大小
    uint32_t rx_buffer_size;        // 接收缓冲区大小
    uint8_t tx_temp_buffer[64];     // 临时发送缓冲区
    uint8_t rx_temp_buffer[64];     // 临时接收缓冲区
} uart_device_t;

// UART设备实例数组
static uart_device_t uart_devices[UART_NUM_MAX] = {0};

// HAL UART句柄映射表（需要用户在具体项目中定义）
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

// HAL UART句柄映射
static UART_HandleTypeDef *get_hal_uart_handle(uart_port_t port)
{
    switch (port)
    {
    case UART_NUM_0:
        return &huart1;
    case UART_NUM_1:
        return &huart2;
    case UART_NUM_2:
        return &huart3;
    default:
        return NULL;
    }
}

// 查找端口号通过HAL句柄
static uart_port_t find_port_by_hal_handle(UART_HandleTypeDef *huart)
{
    for (int i = 0; i < UART_NUM_MAX; i++)
    {
        if (uart_devices[i].hal_uart == huart && uart_devices[i].initialized)
        {
            return (uart_port_t)i;
        }
    }
    return UART_NUM_MAX; // 未找到
}

// Worker任务：处理发送数据（非阻塞版本）
static void uart_tx_worker_task(void *arg)
{
    uart_port_t port = (uart_port_t)(uintptr_t)arg;
    uart_device_t *device = &uart_devices[port];
    size_t bytes_to_send;

    // 检查是否有数据需要发送且当前不忙
    if (device->tx_busy || !device->initialized)
    {
        return; // 发送忙或未初始化，直接返回
    }

    // 从发送流缓冲区获取数据（非阻塞）
    bytes_to_send = xStreamBufferReceive(device->tx_stream, device->tx_temp_buffer,
                                         sizeof(device->tx_temp_buffer), 0);

    if (bytes_to_send > 0)
    {
        device->tx_busy = true;

        // 使用DMA或中断模式发送
        if (device->hal_uart->hdmatx != NULL)
        {
            // DMA模式发送
            HAL_UART_Transmit_DMA(device->hal_uart, device->tx_temp_buffer, bytes_to_send);
        }
        else
        {
            // 中断模式发送
            HAL_UART_Transmit_IT(device->hal_uart, device->tx_temp_buffer, bytes_to_send);
        }
        // 注意：发送完成后会在回调函数中清除tx_busy标志并继续处理下一批数据
    }
}

// 启动接收的Worker任务
static void uart_rx_start_worker_task(void *arg)
{
    uart_port_t port = (uart_port_t)(uintptr_t)arg;
    uart_device_t *device = &uart_devices[port];

    if (!device->initialized)
    {
        return;
    }

    // 启动DMA接收或中断接收
    if (device->hal_uart->hdmarx != NULL)
    {
        HAL_UART_Receive_DMA(device->hal_uart, device->rx_temp_buffer, sizeof(device->rx_temp_buffer));
    }
    else
    {
        HAL_UART_Receive_IT(device->hal_uart, device->rx_temp_buffer, 1);
    }
}

// HAL回调函数：发送完成
static void uart_tx_complete_callback(UART_HandleTypeDef *huart)
{
    // 找到对应的UART设备
    uart_port_t port = find_port_by_hal_handle(huart);
    if (port < UART_NUM_MAX)
    {
        uart_device_t *device = &uart_devices[port];

        // 清除发送忙标志
        device->tx_busy = false;

        // 检查是否还有数据需要发送，如果有则触发新的发送任务
        if (xStreamBufferBytesAvailable(device->tx_stream) > 0)
        {
            worker_queue_item_t tx_item = {
                .cb = uart_tx_worker_task,
                .arg = (void *)(uintptr_t)port,
                .flags = WORKER_FLAG_NONE,
                .name = "UartTxContinue"};
            worker_send(&tx_item);
        }
    }
}

// HAL回调函数：接收完成
static void uart_rx_complete_callback(UART_HandleTypeDef *huart)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // 找到对应的UART设备
    uart_port_t port = find_port_by_hal_handle(huart);
    if (port < UART_NUM_MAX)
    {
        uart_device_t *device = &uart_devices[port];

        if (device->hal_uart->hdmarx != NULL)
        {
            // DMA模式：将整个缓冲区的数据写入流缓冲区
            size_t received_bytes = sizeof(device->rx_temp_buffer) - __HAL_DMA_GET_COUNTER(huart->hdmarx);
            if (received_bytes > 0)
            {
                xStreamBufferSendFromISR(device->rx_stream, device->rx_temp_buffer,
                                         received_bytes, &xHigherPriorityTaskWoken);
            }

            // 重新启动DMA接收
            HAL_UART_Receive_DMA(huart, device->rx_temp_buffer, sizeof(device->rx_temp_buffer));
        }
        else
        {
            // 中断模式：单字节接收
            xStreamBufferSendFromISR(device->rx_stream, device->rx_temp_buffer, 1, &xHigherPriorityTaskWoken);

            // 继续接收下一个字节
            HAL_UART_Receive_IT(huart, device->rx_temp_buffer, 1);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// HAL回调函数：接收半完成
static void uart_rx_half_complete_callback(UART_HandleTypeDef *huart)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // 找到对应的UART设备
    uart_port_t port = find_port_by_hal_handle(huart);
    if (port < UART_NUM_MAX)
    {
        uart_device_t *device = &uart_devices[port];

        // DMA半完成：处理前半部分数据
        size_t half_size = sizeof(device->rx_temp_buffer) / 2;
        xStreamBufferSendFromISR(device->rx_stream, device->rx_temp_buffer,
                                 half_size, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// HAL回调函数：错误处理
static void uart_error_callback(UART_HandleTypeDef *huart)
{
    // 找到对应的UART设备
    uart_port_t port = find_port_by_hal_handle(huart);
    if (port < UART_NUM_MAX)
    {
        uart_device_t *device = &uart_devices[port];

        // 清除发送忙标志
        device->tx_busy = false;

        // 重新启动接收 - 通过Worker任务处理
        worker_queue_item_t rx_item = {
            .cb = uart_rx_start_worker_task,
            .arg = (void *)(uintptr_t)port,
            .flags = WORKER_FLAG_HIGH_PRIO,
            .name = "UartRxRestart"};

        worker_send(&rx_item);
    }
}

// 初始化UART
esp_err_t uart_async_init(uart_port_t port, uint32_t size)
{
    if (port >= UART_NUM_MAX)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uart_device_t *device = &uart_devices[port];

    if (device->initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // 获取HAL UART句柄
    device->hal_uart = get_hal_uart_handle(port);
    if (device->hal_uart == NULL)
    {
        return ESP_ERR_NOT_FOUND;
    }

    // 创建流缓冲区
    device->tx_buffer_size = size;
    device->rx_buffer_size = size;

    device->tx_stream = xStreamBufferCreate(size, 1);
    device->rx_stream = xStreamBufferCreate(size, 1);

    if (device->tx_stream == NULL || device->rx_stream == NULL)
    {
        // 清理已创建的资源
        if (device->tx_stream)
            vStreamBufferDelete(device->tx_stream);
        if (device->rx_stream)
            vStreamBufferDelete(device->rx_stream);
        return ESP_ERR_NO_MEM;
    }

    // 创建互斥锁
    device->tx_mutex = xSemaphoreCreateMutex();
    device->rx_mutex = xSemaphoreCreateMutex();

    if (device->tx_mutex == NULL || device->rx_mutex == NULL)
    {
        // 清理资源
        vStreamBufferDelete(device->tx_stream);
        vStreamBufferDelete(device->rx_stream);
        if (device->tx_mutex)
            vSemaphoreDelete(device->tx_mutex);
        if (device->rx_mutex)
            vSemaphoreDelete(device->rx_mutex);
        return ESP_ERR_NO_MEM;
    }

    device->tx_busy = false;
    device->initialized = true;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    // 注册HAL回调函数
    HAL_UART_RegisterCallback(device->hal_uart, HAL_UART_TX_COMPLETE_CB_ID, uart_tx_complete_callback);
    HAL_UART_RegisterCallback(device->hal_uart, HAL_UART_RX_COMPLETE_CB_ID, uart_rx_complete_callback);
    HAL_UART_RegisterCallback(device->hal_uart, HAL_UART_RX_HALFCOMPLETE_CB_ID, uart_rx_half_complete_callback);
    HAL_UART_RegisterCallback(device->hal_uart, HAL_UART_ERROR_CB_ID, uart_error_callback);
#endif

    // 启动接收
    worker_queue_item_t rx_item = {
        .cb = uart_rx_start_worker_task,
        .arg = (void *)(uintptr_t)port,
        .flags = WORKER_FLAG_NONE,
        .name = "UartRxStart"};

    if (worker_send(&rx_item) != 0)
    {
        // 清理资源
        vStreamBufferDelete(device->tx_stream);
        vStreamBufferDelete(device->rx_stream);
        vSemaphoreDelete(device->tx_mutex);
        vSemaphoreDelete(device->rx_mutex);
        device->initialized = false;
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

// 发送数据
int uart_write_bytes(uart_port_t uart_num, const void *src, size_t size)
{
    if (uart_num >= UART_NUM_MAX || src == NULL || size == 0)
    {
        return -1;
    }

    uart_device_t *device = &uart_devices[uart_num];

    if (!device->initialized)
    {
        return -1;
    }

    // 获取发送互斥锁
    if (xSemaphoreTake(device->tx_mutex, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
        return -1;
    }

    // 发送数据到流缓冲区
    size_t bytes_sent = xStreamBufferSend(device->tx_stream, src, size, pdMS_TO_TICKS(1000));

    xSemaphoreGive(device->tx_mutex);

    // 触发发送处理（如果当前不忙）
    if (bytes_sent > 0 && !device->tx_busy)
    {
        worker_queue_item_t tx_item = {
            .cb = uart_tx_worker_task,
            .arg = (void *)(uintptr_t)uart_num,
            .flags = WORKER_FLAG_NONE,
            .name = "UartTx"};
        worker_send(&tx_item);
    }

    return (int)bytes_sent;
}

// 接收数据
int uart_read_bytes(uart_port_t uart_num, void *buf, uint32_t length, TickType_t ticks_to_wait)
{
    if (uart_num >= UART_NUM_MAX || buf == NULL || length == 0)
    {
        return -1;
    }

    uart_device_t *device = &uart_devices[uart_num];

    if (!device->initialized)
    {
        return -1;
    }

    // 从接收流缓冲区读取数据
    size_t bytes_read = xStreamBufferReceive(device->rx_stream, buf, length, ticks_to_wait);

    return (int)bytes_read;
}

// 等待发送完成
esp_err_t uart_flush(uart_port_t uart_num)
{
    if (uart_num >= UART_NUM_MAX)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uart_device_t *device = &uart_devices[uart_num];

    if (!device->initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // 等待流缓冲区为空且发送不忙
    while (xStreamBufferBytesAvailable(device->tx_stream) > 0 || device->tx_busy)
    {
        vTaskDelay(pdMS_TO_TICKS(1));

        // 如果有剩余数据且发送不忙，触发发送
        if (xStreamBufferBytesAvailable(device->tx_stream) > 0 && !device->tx_busy)
        {
            worker_queue_item_t tx_item = {
                .cb = uart_tx_worker_task,
                .arg = (void *)(uintptr_t)uart_num,
                .flags = WORKER_FLAG_NONE,
                .name = "UartTxFlush"};
            worker_send(&tx_item);
        }
    }

    return ESP_OK;
}

// 清空缓冲区
esp_err_t uart_clear(uart_port_t uart_num)
{
    if (uart_num >= UART_NUM_MAX)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uart_device_t *device = &uart_devices[uart_num];

    if (!device->initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // 清空发送和接收缓冲区
    xStreamBufferReset(device->tx_stream);
    xStreamBufferReset(device->rx_stream);

    return ESP_OK;
}

// 获取接收缓冲区数据长度
esp_err_t uart_get_buffered_data_len(uart_port_t uart_num, size_t *size)
{
    if (uart_num >= UART_NUM_MAX || size == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uart_device_t *device = &uart_devices[uart_num];

    if (!device->initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    *size = xStreamBufferBytesAvailable(device->rx_stream);
    return ESP_OK;
}

// 获取发送缓冲区空闲空间
esp_err_t uart_get_tx_buffer_free_size(uart_port_t uart_num, size_t *size)
{
    if (uart_num >= UART_NUM_MAX || size == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uart_device_t *device = &uart_devices[uart_num];

    if (!device->initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    *size = xStreamBufferSpacesAvailable(device->tx_stream);
    return ESP_OK;
}

int uart3_init(void *arg)
{
    return uart_async_init(UART_NUM_2, 1024);
}
#include "stdlib.h"
#include "log.h"
#include "periph_init.h"
PERIPH_INIT_REGISTER("uart3_init", 300, uart3_init, NULL);

void uart3_test()
{
    char buf[512];
    memset(buf, 'A', sizeof(buf));
    if (uart_write_bytes(UART_NUM_2, buf, sizeof(buf)) < 0)
    {
        loge("err");
    }
}