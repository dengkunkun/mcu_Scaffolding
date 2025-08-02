/**
 * @file worker_example.c
 * @brief 简化版Worker线程使用示例
 *
 * 演示了如何在STM32 FreeRTOS项目中使用简化版worker线程
 * 只包括基本任务提交和高优先级任务功能
 */

#include "worker.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
// 示例数据结构
typedef struct
{
    uint32_t sensor_id;
    float value;
    uint32_t timestamp;
} sensor_data_t;

typedef struct
{
    uint8_t led_id;
    bool state;
} led_control_t;

// =============================================================================
// 工作回调函数示例
// =============================================================================

/**
 * @brief 传感器数据处理工作函数
 */
void sensor_data_process_work(void *arg)
{
    sensor_data_t *data = (sensor_data_t *)arg;

    if (!data)
    {
        printf("Error: Invalid sensor data\n");
        return;
    }

    // 模拟数据处理
    printf("Processing sensor %lu: value=%.2f, time=%lu\n",
           data->sensor_id, data->value, data->timestamp);

    // 数据验证
    if (data->value > 100.0f)
    {
        printf("Warning: Sensor %lu value %.2f exceeds threshold\n",
               data->sensor_id, data->value);
    }

    // 释放动态分配的数据
    vPortFree(data);
}

/**
 * @brief LED控制工作函数
 */
void led_control_work(void *arg)
{
    led_control_t *led = (led_control_t *)arg;

    if (!led)
    {
        return;
    }

    printf("LED %d -> %s\n", led->led_id, led->state ? "ON" : "OFF");

    // 实际的LED控制代码
    // HAL_GPIO_WritePin(LED_GPIO_Port, led->led_id, led->state ? GPIO_PIN_SET : GPIO_PIN_RESET);

    vPortFree(led);
}

/**
 * @brief 系统状态监控工作函数（高优先级）
 */
void system_monitor_work(void *arg)
{
    (void)arg;

    // 检查系统资源
    UBaseType_t min_heap = xPortGetMinimumEverFreeHeapSize();
    UBaseType_t free_heap = xPortGetFreeHeapSize();

    printf("System Monitor - Free heap: %u bytes, Min heap: %u bytes\n",
           free_heap, min_heap);

    // 检查任务栈使用情况
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    UBaseType_t stack_watermark = uxTaskGetStackHighWaterMark(current_task);

    printf("Worker stack watermark: %u words\n", stack_watermark);
}

/**
 * @brief 网络数据发送工作函数
 */
void network_send_work(void *arg)
{
    char *message = (char *)arg;

    if (!message)
    {
        return;
    }

    printf("Network: Sending: %s\n", message);

    // 模拟网络发送
    vTaskDelay(pdMS_TO_TICKS(100)); // 模拟网络延迟

    printf("Network: Send complete: %s\n", message);
    vPortFree(message);
}

/**
 * @brief 紧急处理函数（高优先级）
 */
void emergency_handler(void *arg)
{
    uint32_t *error_code = (uint32_t *)arg;

    printf("EMERGENCY: Processing error code 0x%08lX\n", *error_code);

    // 紧急处理逻辑
    switch (*error_code)
    {
    case 0x01:
        printf("EMERGENCY: Hardware fault detected!\n");
        break;
    case 0x02:
        printf("EMERGENCY: Communication timeout!\n");
        break;
    default:
        printf("EMERGENCY: Unknown error!\n");
        break;
    }

    vPortFree(error_code);
}

// =============================================================================
// 使用示例函数
// =============================================================================

/**
 * @brief 基本worker任务提交示例
 */
void worker_basic_example(void)
{
    printf("=== Basic Worker Example ===\n");

    // 提交传感器数据处理任务
    for (int i = 0; i < 3; i++)
    {
        sensor_data_t *data = pvPortMalloc(sizeof(sensor_data_t));
        if (data)
        {
            data->sensor_id = i + 1;
            data->value = 25.5f + i * 10.0f;
            data->timestamp = xTaskGetTickCount();

            WORKER_SUBMIT(sensor_data_process_work, data);
        }
    }

    // 提交LED控制任务
    led_control_t *led = pvPortMalloc(sizeof(led_control_t));
    if (led)
    {
        led->led_id = 1;
        led->state = true;
        WORKER_SUBMIT(led_control_work, led);
    }

    // 提交高优先级系统监控任务
    WORKER_SUBMIT_HIGH_PRIO(system_monitor_work, NULL);
}

/**
 * @brief 高优先级任务示例
 */
void worker_priority_example(void)
{
    printf("=== Priority Worker Example ===\n");

    // 先提交一些普通任务
    for (int i = 0; i < 5; i++)
    {
        char *message = pvPortMalloc(64);
        if (message)
        {
            snprintf(message, 64, "Normal priority message #%d", i + 1);
            WORKER_SUBMIT(network_send_work, message);
        }
    }

    // 然后提交高优先级紧急任务
    uint32_t *error_code = pvPortMalloc(sizeof(uint32_t));
    if (error_code)
    {
        *error_code = 0x01; // 硬件故障
        WORKER_SUBMIT_HIGH_PRIO(emergency_handler, error_code);
    }

    printf("Submitted normal tasks and emergency task\n");
    printf("Emergency task will be processed first due to high priority\n");
}

/**
 * @brief 批量任务处理示例
 */
void worker_batch_example(void)
{
    printf("=== Batch Worker Example ===\n");

    const int batch_size = 10;

    // 提交一批网络发送任务
    for (int i = 0; i < batch_size; i++)
    {
        char *message = pvPortMalloc(64);
        if (message)
        {
            snprintf(message, 64, "Batch message #%d", i + 1);
            WORKER_SUBMIT(network_send_work, message);
        }
    }

    printf("Submitted %d batch tasks\n", batch_size);

    // 等待所有任务完成
    printf("Waiting for all tasks to complete...\n");
    if (worker_flush(30000) == 0)
    { // 30秒超时
        printf("All batch tasks completed\n");
    }
    else
    {
        printf("Timeout waiting for batch tasks\n");
    }
}

/**
 * @brief Worker状态监控示例
 */
void worker_monitoring_example(void)
{
    printf("=== Worker Monitoring Example ===\n");

    // 打印worker状态
    worker_print_status();

    // 获取当前队列长度
    uint32_t queue_len = worker_get_queue_length();
    printf("Current queue length: %lu\n", queue_len);

    // 获取worker状态
    worker_state_t state = worker_get_state();
    printf("Worker state: %s\n",
           state == WORKER_STATE_RUNNING ? "RUNNING" : state == WORKER_STATE_SUSPENDED ? "SUSPENDED"
                                                   : state == WORKER_STATE_STOPPED     ? "STOPPED"
                                                                                       : "ERROR");
}

/**
 * @brief 完整的worker使用示例
 */
void worker_complete_example(void)
{
    printf("Starting Simplified Worker Thread Examples...\n");

    // 初始化worker线程
    // 参数：队列大小=32, 栈大小=1536字节, 优先级=8
    if (worker_thread_init(32, 1536, 8) != 0)
    {
        printf("Failed to initialize worker thread\n");
        return;
    }

    printf("Worker thread initialized successfully\n");

    // 运行各种示例
    worker_basic_example();
    vTaskDelay(pdMS_TO_TICKS(2000));

    worker_monitoring_example();
    vTaskDelay(pdMS_TO_TICKS(1000));

    worker_priority_example();
    vTaskDelay(pdMS_TO_TICKS(3000));

    worker_batch_example();
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 最终状态检查
    worker_monitoring_example();

    printf("Worker examples completed\n");
}

// =============================================================================
// 集成到你的项目中的建议
// =============================================================================

/**
 * @brief 在app_main.c中的集成示例
 */
void integrate_worker_into_app_main(void)
{
    // 在app_main任务中初始化worker线程
    if (worker_thread_init(32, 1536, 8) == 0)
    {
        printf("Worker thread ready\n");

        // 提交系统启动后的初始化任务
        WORKER_SUBMIT(system_monitor_work, NULL);
    }
    else
    {
        printf("Worker thread init failed\n");
    }
}

/**
 * @brief 在中断或回调中使用worker的示例
 */
void uart_rx_callback_example(void)
{
    // 在UART接收中断中，不要直接处理数据
    // 而是提交给worker线程处理

    static uint8_t rx_buffer[256];
    static uint16_t rx_length = 100; // 假设接收到100字节

    // 创建数据副本并提交给worker处理
    uint8_t *data_copy = pvPortMalloc(rx_length);
    if (data_copy)
    {
        memcpy(data_copy, rx_buffer, rx_length);

        // 使用高优先级确保及时处理
        worker_queue_item_t item = {
            .cb = network_send_work,
            .arg = data_copy,
            .flags = WORKER_FLAG_HIGH_PRIO,
            .name = "UartRxProcess"};

        worker_send(&item);
    }
}

/**
 * @brief 错误处理中使用worker的示例
 */
void error_handler_example(uint32_t error_code)
{
    // 在错误处理函数中，使用高优先级任务处理错误
    uint32_t *error_data = pvPortMalloc(sizeof(uint32_t));
    if (error_data)
    {
        *error_data = error_code;
        WORKER_SUBMIT_HIGH_PRIO(emergency_handler, error_data);
    }
}