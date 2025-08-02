#include "unity.h"
#include "worker.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// 测试数据结构
typedef struct
{
    int value;
    bool executed;
    uint32_t execution_time;
} test_work_data_t;

// 全局测试变量
static test_work_data_t g_test_data;
static SemaphoreHandle_t g_test_sem;
static volatile int g_callback_count = 0;

// 测试回调函数
void test_work_callback(void *arg)
{
    test_work_data_t *data = (test_work_data_t *)arg;
    if (data)
    {
        data->executed = true;
        data->execution_time = xTaskGetTickCount();
        g_callback_count++;
    }

    if (g_test_sem)
    {
        xSemaphoreGive(g_test_sem);
    }
}

void simple_work_callback(void *arg)
{
    int *counter = (int *)arg;
    if (counter)
    {
        (*counter)++;
    }
}

void high_priority_work_callback(void *arg)
{
    int *order = (int *)arg;
    if (order)
    {
        (*order) = 1; // 高优先级任务应该先执行
    }
}

void normal_priority_work_callback(void *arg)
{
    int *order = (int *)arg;
    if (order && *order == 0)
    {
        (*order) = 2; // 如果高优先级还没执行，这个会是2
    }
}

// setUp和tearDown函数
void setUp(void)
{
    // 每个测试前的设置
    memset(&g_test_data, 0, sizeof(g_test_data));
    g_callback_count = 0;

    // 创建测试信号量
    g_test_sem = xSemaphoreCreateBinary();
    TEST_ASSERT_NOT_NULL(g_test_sem);
}

void tearDown(void)
{
    // 每个测试后的清理
    if (g_test_sem)
    {
        vSemaphoreDelete(g_test_sem);
        g_test_sem = NULL;
    }

    // 清理Worker（如果已初始化）
    worker_thread_destroy();
}

// 测试用例：Worker初始化
void test_worker_init_success(void)
{
    int result = worker_thread_init(10, 1024, 5);
    TEST_ASSERT_EQUAL_INT(0, result);

    worker_state_t state = worker_get_state();
    TEST_ASSERT_EQUAL_INT(WORKER_STATE_RUNNING, state);
}

void test_worker_init_already_initialized(void)
{
    // 第一次初始化应该成功
    int result1 = worker_thread_init(10, 1024, 5);
    TEST_ASSERT_EQUAL_INT(0, result1);

    // 第二次初始化应该失败
    int result2 = worker_thread_init(10, 1024, 5);
    TEST_ASSERT_EQUAL_INT(-1, result2);
}

// 测试用例：基本任务提交
void test_worker_submit_basic_task(void)
{
    // 初始化Worker
    int result = worker_thread_init(10, 1024, 5);
    TEST_ASSERT_EQUAL_INT(0, result);

    // 设置测试数据
    g_test_data.value = 42;
    g_test_data.executed = false;

    // 提交任务
    worker_queue_item_t item = {
        .cb = test_work_callback,
        .arg = &g_test_data,
        .flags = WORKER_FLAG_NONE,
        .name = "test_task"};

    result = worker_send(&item);
    TEST_ASSERT_EQUAL_INT(0, result);

    // 等待任务执行完成
    BaseType_t sem_result = xSemaphoreTake(g_test_sem, pdMS_TO_TICKS(1000));
    TEST_ASSERT_EQUAL_INT(pdTRUE, sem_result);

    // 验证任务已执行
    TEST_ASSERT_TRUE(g_test_data.executed);
    TEST_ASSERT_EQUAL_INT(42, g_test_data.value);
    TEST_ASSERT_EQUAL_INT(1, g_callback_count);
}

// 测试用例：高优先级任务
void test_worker_high_priority_task(void)
{
    int result = worker_thread_init(10, 1024, 5);
    TEST_ASSERT_EQUAL_INT(0, result);

    static int execution_order = 0;

    // 先提交普通优先级任务
    worker_queue_item_t normal_item = {
        .cb = normal_priority_work_callback,
        .arg = &execution_order,
        .flags = WORKER_FLAG_NONE,
        .name = "normal_task"};

    // 再提交高优先级任务
    worker_queue_item_t high_item = {
        .cb = high_priority_work_callback,
        .arg = &execution_order,
        .flags = WORKER_FLAG_HIGH_PRIO,
        .name = "high_prio_task"};

    result = worker_send(&normal_item);
    TEST_ASSERT_EQUAL_INT(0, result);

    result = worker_send(&high_item);
    TEST_ASSERT_EQUAL_INT(0, result);

    // 等待执行完成
    vTaskDelay(pdMS_TO_TICKS(100));

    // 高优先级任务应该先执行
    TEST_ASSERT_EQUAL_INT(1, execution_order);
}

// 测试用例：批量任务处理
void test_worker_batch_tasks(void)
{
    int result = worker_thread_init(20, 1024, 5);
    TEST_ASSERT_EQUAL_INT(0, result);

    static int counter = 0;
    const int task_count = 5;

    // 提交多个任务
    for (int i = 0; i < task_count; i++)
    {
        worker_queue_item_t item = {
            .cb = simple_work_callback,
            .arg = &counter,
            .flags = WORKER_FLAG_NONE,
            .name = "batch_task"};

        result = worker_send(&item);
        TEST_ASSERT_EQUAL_INT(0, result);
    }

    // 等待所有任务完成
    result = worker_flush(2000);
    TEST_ASSERT_EQUAL_INT(0, result);

    // 验证所有任务都执行了
    TEST_ASSERT_EQUAL_INT(task_count, counter);
}

// 测试用例：Worker状态管理
void test_worker_state_management(void)
{
    // 初始状态
    worker_state_t state = worker_get_state();
    TEST_ASSERT_EQUAL_INT(WORKER_STATE_ERROR, state);

    // 初始化后状态
    int result = worker_thread_init(10, 1024, 5);
    TEST_ASSERT_EQUAL_INT(0, result);

    state = worker_get_state();
    TEST_ASSERT_EQUAL_INT(WORKER_STATE_RUNNING, state);

    // 暂停状态
    result = worker_suspend();
    TEST_ASSERT_EQUAL_INT(0, result);

    state = worker_get_state();
    TEST_ASSERT_EQUAL_INT(WORKER_STATE_SUSPENDED, state);

    // 恢复状态
    result = worker_resume();
    TEST_ASSERT_EQUAL_INT(0, result);

    state = worker_get_state();
    TEST_ASSERT_EQUAL_INT(WORKER_STATE_RUNNING, state);
}

// 测试用例：队列长度检查
void test_worker_queue_length(void)
{
    int result = worker_thread_init(5, 1024, 5);
    TEST_ASSERT_EQUAL_INT(0, result);

    // 初始队列长度应该为0
    uint32_t length = worker_get_queue_length();
    TEST_ASSERT_EQUAL_UINT32(0, length);

    // 提交一个会阻塞的任务（暂停Worker）
    worker_suspend();

    worker_queue_item_t item = {
        .cb = simple_work_callback,
        .arg = NULL,
        .flags = WORKER_FLAG_NONE,
        .name = "queue_test"};

    result = worker_send(&item);
    TEST_ASSERT_EQUAL_INT(0, result);

    length = worker_get_queue_length();
    TEST_ASSERT_EQUAL_UINT32(1, length);

    // 恢复执行
    worker_resume();
    vTaskDelay(pdMS_TO_TICKS(50));

    length = worker_get_queue_length();
    TEST_ASSERT_EQUAL_UINT32(0, length);
}

// 测试用例：Worker销毁
void test_worker_destroy(void)
{
    int result = worker_thread_init(10, 1024, 5);
    TEST_ASSERT_EQUAL_INT(0, result);

    worker_state_t state = worker_get_state();
    TEST_ASSERT_EQUAL_INT(WORKER_STATE_RUNNING, state);

    result = worker_thread_destroy();
    TEST_ASSERT_EQUAL_INT(0, result);

    state = worker_get_state();
    TEST_ASSERT_EQUAL_INT(WORKER_STATE_ERROR, state);
}

// 主测试运行器
void worker_test_runner(void)
{
    UNITY_BEGIN();

    printf("=== Worker Component Test Suite ===\n");

    // 基础功能测试
    RUN_TEST(test_worker_init_success);
    RUN_TEST(test_worker_init_already_initialized);
    RUN_TEST(test_worker_submit_basic_task);

    // 高级功能测试
    RUN_TEST(test_worker_high_priority_task);
    RUN_TEST(test_worker_batch_tasks);

    // 状态管理测试
    RUN_TEST(test_worker_state_management);
    RUN_TEST(test_worker_queue_length);

    // 清理测试
    RUN_TEST(test_worker_destroy);

    UNITY_END();
}

// 如果是独立测试，提供main函数
#ifdef WORKER_TEST_STANDALONE
int main(void)
{
    worker_test_runner();
    return 0;
}
#endif