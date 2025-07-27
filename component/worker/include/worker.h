#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Worker线程状态
    typedef enum
    {
        WORKER_STATE_STOPPED = 0,
        WORKER_STATE_RUNNING,
        WORKER_STATE_SUSPENDED,
        WORKER_STATE_ERROR
    } worker_state_t;

    // Worker任务标志
    typedef enum
    {
        WORKER_FLAG_NONE = 0x00,
        WORKER_FLAG_HIGH_PRIO = 0x01 // 高优先级任务（插入队列头部）
    } worker_flags_t;

    // 工作回调函数类型
    typedef void (*worker_cb_t)(void *arg);

    // 工作队列项结构
    typedef struct
    {
        worker_cb_t cb;   // 工作回调函数
        void *arg;        // 回调函数参数
        uint32_t flags;   // 工作标志
        const char *name; // 任务名称（调试用）
    } worker_queue_item_t;

    // =============================================================================
    // 基础Worker接口
    // =============================================================================

    /**
     * @brief 初始化worker线程
     * @param item_num 工作队列最大项数
     * @param stack_size 线程栈大小
     * @param prior 线程优先级
     * @return 0:成功 -1:失败
     */
    int worker_thread_init(int item_num, int stack_size, int prior);

    /**
     * @brief 销毁worker线程
     * @return 0:成功 -1:失败
     */
    int worker_thread_destroy(void);

    /**
     * @brief 发送工作任务到队列
     * @param item 工作项指针
     * @return 0:成功 -1:失败
     */
    int worker_send(worker_queue_item_t *item);

    /**
     * @brief 发送工作任务到队列（带超时）
     * @param item 工作项指针
     * @param timeout_ms 超时时间（毫秒）
     * @return 0:成功 -1:超时 -2:其他错误
     */
    int worker_send_timeout(worker_queue_item_t *item, uint32_t timeout_ms);

    /**
     * @brief 刷新工作队列（等待所有任务完成）
     * @param timeout_ms 超时时间（毫秒）
     * @return 0:成功 -1:超时
     */
    int worker_flush(uint32_t timeout_ms);

// =============================================================================
// 便利宏定义
// =============================================================================

/**
 * @brief 简单工作任务宏
 * @param func 函数名
 * @param data 参数
 */
#define WORKER_SUBMIT(func, data)      \
    do                                 \
    {                                  \
        worker_queue_item_t item = {   \
            .cb = (func),              \
            .arg = (data),             \
            .flags = WORKER_FLAG_NONE, \
            .name = #func};            \
        worker_send(&item);            \
    } while (0)

/**
 * @brief 高优先级工作任务宏
 * @param func 函数名
 * @param data 参数
 */
#define WORKER_SUBMIT_HIGH_PRIO(func, data) \
    do                                      \
    {                                       \
        worker_queue_item_t item = {        \
            .cb = (func),                   \
            .arg = (data),                  \
            .flags = WORKER_FLAG_HIGH_PRIO, \
            .name = #func};                 \
        worker_send(&item);                 \
    } while (0)

    // =============================================================================
    // 状态和调试接口
    // =============================================================================

    /**
     * @brief 获取worker状态
     * @return worker_state_t 当前状态
     */
    worker_state_t worker_get_state(void);

    /**
     * @brief 打印worker状态（调试用）
     */
    void worker_print_status(void);

    /**
     * @brief 获取当前队列长度
     * @return 队列中等待的任务数
     */
    uint32_t worker_get_queue_length(void);

    /**
     * @brief 暂停worker线程
     * @return 0:成功 -1:失败
     */
    int worker_suspend(void);

    /**
     * @brief 恢复worker线程
     * @return 0:成功 -1:失败
     */
    int worker_resume(void);

#ifdef __cplusplus
}
#endif