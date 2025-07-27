#include "worker.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// 内部配置
#define WORKER_TASK_NAME "WorkerThread"
#define WORKER_MAGIC 0x574B // "WK" 魔数

// Worker内部控制结构
typedef struct
{
    TaskHandle_t task_handle;    // 主工作线程句柄
    QueueHandle_t work_queue;    // 工作队列
    SemaphoreHandle_t flush_sem; // 刷新信号量

    // 状态标志
    uint16_t magic;                   // 魔数，用于检查初始化状态
    volatile worker_state_t state;    // Worker状态
    volatile bool shutdown_requested; // 关闭请求标志
    volatile bool flush_requested;    // 刷新请求标志
} worker_control_t;

// 全局worker控制结构
static worker_control_t g_worker = {
    .magic = 0,
    .state = WORKER_STATE_STOPPED,
    .shutdown_requested = false,
    .flush_requested = false};

// =============================================================================
// 内部辅助函数
// =============================================================================

/**
 * @brief 执行工作项
 */
static void execute_work_item(const worker_queue_item_t *item)
{
    if (!item || !item->cb)
    {
        return;
    }

    // 执行工作回调
    item->cb(item->arg);
}

/**
 * @brief 高优先级任务插入队列头部
 */
static BaseType_t send_to_queue_front(QueueHandle_t queue, const worker_queue_item_t *item, TickType_t timeout)
{
    return xQueueSendToFront(queue, item, timeout);
}

/**
 * @brief 主工作线程函数
 */
static void worker_thread_function(void *param)
{
    (void)param;

    worker_queue_item_t work_item;
    BaseType_t result;

    g_worker.state = WORKER_STATE_RUNNING;

    while (!g_worker.shutdown_requested)
    {
        // 从队列接收工作项，50ms超时
        result = xQueueReceive(g_worker.work_queue, &work_item, pdMS_TO_TICKS(50));

        if (result == pdTRUE)
        {
            // 执行工作项
            execute_work_item(&work_item);

            // 检查是否需要刷新信号
            if (g_worker.flush_requested)
            {
                uint32_t queue_length = uxQueueMessagesWaiting(g_worker.work_queue);
                if (queue_length == 0)
                {
                    g_worker.flush_requested = false;
                    xSemaphoreGive(g_worker.flush_sem);
                }
            }
        }
    }

    g_worker.state = WORKER_STATE_STOPPED;
    vTaskDelete(NULL);
}

// =============================================================================
// 公共接口实现
// =============================================================================

int worker_thread_init(int item_num, int stack_size, int prior)
{
    if (g_worker.magic == WORKER_MAGIC)
    {
        return -1; // 已经初始化
    }

    // 创建工作队列
    g_worker.work_queue = xQueueCreate(item_num, sizeof(worker_queue_item_t));
    if (!g_worker.work_queue)
    {
        return -1;
    }

    // 创建刷新信号量
    g_worker.flush_sem = xSemaphoreCreateBinary();
    if (!g_worker.flush_sem)
    {
        vQueueDelete(g_worker.work_queue);
        return -1;
    }

    // 创建工作线程
    BaseType_t result = xTaskCreate(
        worker_thread_function,
        WORKER_TASK_NAME,
        stack_size / sizeof(StackType_t),
        NULL,
        prior,
        &g_worker.task_handle);

    if (result != pdPASS)
    {
        vQueueDelete(g_worker.work_queue);
        vSemaphoreDelete(g_worker.flush_sem);
        return -1;
    }

    // 初始化状态
    g_worker.state = WORKER_STATE_RUNNING;
    g_worker.shutdown_requested = false;
    g_worker.flush_requested = false;

    // 设置魔数表示初始化完成
    g_worker.magic = WORKER_MAGIC;

    return 0;
}

int worker_thread_destroy(void)
{
    if (g_worker.magic != WORKER_MAGIC)
    {
        return -1;
    }

    // 请求关闭
    g_worker.shutdown_requested = true;

    // 等待线程结束
    while (eTaskGetState(g_worker.task_handle) != eDeleted)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // 删除队列和信号量
    vQueueDelete(g_worker.work_queue);
    vSemaphoreDelete(g_worker.flush_sem);

    // 清除魔数和状态
    g_worker.magic = 0;
    g_worker.state = WORKER_STATE_STOPPED;

    return 0;
}

int worker_send(worker_queue_item_t *item)
{
    if (g_worker.magic != WORKER_MAGIC || !item)
    {
        return -1;
    }

    BaseType_t result;

    // 根据优先级标志决定插入位置
    if (item->flags & WORKER_FLAG_HIGH_PRIO)
    {
        // 高优先级任务插入队列头部
        result = send_to_queue_front(g_worker.work_queue, item, 0);
    }
    else
    {
        // 普通任务插入队列尾部
        result = xQueueSend(g_worker.work_queue, item, 0);
    }

    return (result == pdTRUE) ? 0 : -1;
}

int worker_send_timeout(worker_queue_item_t *item, uint32_t timeout_ms)
{
    if (g_worker.magic != WORKER_MAGIC || !item)
    {
        return -2;
    }

    BaseType_t result;
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    // 根据优先级标志决定插入位置
    if (item->flags & WORKER_FLAG_HIGH_PRIO)
    {
        // 高优先级任务插入队列头部
        result = send_to_queue_front(g_worker.work_queue, item, timeout_ticks);
    }
    else
    {
        // 普通任务插入队列尾部
        result = xQueueSend(g_worker.work_queue, item, timeout_ticks);
    }

    if (result == errQUEUE_FULL)
    {
        return -1; // 超时
    }
    else if (result != pdTRUE)
    {
        return -2; // 其他错误
    }

    return 0;
}

int worker_flush(uint32_t timeout_ms)
{
    if (g_worker.magic != WORKER_MAGIC)
    {
        return -1;
    }

    // 检查队列是否已经为空
    uint32_t queue_length = uxQueueMessagesWaiting(g_worker.work_queue);
    if (queue_length == 0)
    {
        return 0;
    }

    // 设置刷新请求
    g_worker.flush_requested = true;

    // 等待刷新完成
    BaseType_t result = xSemaphoreTake(g_worker.flush_sem,
                                       pdMS_TO_TICKS(timeout_ms));

    return (result == pdTRUE) ? 0 : -1;
}

worker_state_t worker_get_state(void)
{
    if (g_worker.magic != WORKER_MAGIC)
    {
        return WORKER_STATE_ERROR;
    }

    return g_worker.state;
}

void worker_print_status(void)
{
    if (g_worker.magic != WORKER_MAGIC)
    {
        printf("Worker not initialized\n");
        return;
    }

    printf("=== Worker Thread Status ===\n");
    printf("State: %s\n",
           g_worker.state == WORKER_STATE_RUNNING ? "RUNNING" : g_worker.state == WORKER_STATE_SUSPENDED ? "SUSPENDED"
                                                            : g_worker.state == WORKER_STATE_STOPPED     ? "STOPPED"
                                                                                                         : "ERROR");

    UBaseType_t messages_waiting = uxQueueMessagesWaiting(g_worker.work_queue);
    UBaseType_t queue_spaces = uxQueueSpacesAvailable(g_worker.work_queue);
    UBaseType_t queue_length = messages_waiting + queue_spaces;

    printf("Queue length: %lu/%lu\n",
           (unsigned long)messages_waiting,
           (unsigned long)queue_length);
    printf("Free heap: %u bytes\n", xPortGetFreeHeapSize());
}

uint32_t worker_get_queue_length(void)
{
    if (g_worker.magic != WORKER_MAGIC)
    {
        return 0;
    }

    return uxQueueMessagesWaiting(g_worker.work_queue);
}

int worker_suspend(void)
{
    if (g_worker.magic != WORKER_MAGIC)
    {
        return -1;
    }

    vTaskSuspend(g_worker.task_handle);
    g_worker.state = WORKER_STATE_SUSPENDED;

    return 0;
}

int worker_resume(void)
{
    if (g_worker.magic != WORKER_MAGIC)
    {
        return -1;
    }

    vTaskResume(g_worker.task_handle);
    g_worker.state = WORKER_STATE_RUNNING;

    return 0;
}