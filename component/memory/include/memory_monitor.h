/**
 * @file memory_monitor.h
 * @brief STM32内存使用监控工具
 *
 * 提供运行时内存使用情况分析，包括堆栈使用、全局变量占用等
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // 内存区域信息结构
    typedef struct
    {
        uint32_t total_ram;   // 总RAM大小
        uint32_t total_flash; // 总Flash大小

        // RAM区域使用情况
        uint32_t data_size;          // .data段大小（初始化全局变量）
        uint32_t bss_size;           // .bss段大小（未初始化全局变量）
        uint32_t freertos_heap_size; // 堆区域大小
        uint32_t libc_heap_size;
        uint32_t libc_heap_start;
        uint32_t stack_size; // 栈区域大小
        uint32_t free_ram;   // 剩余可用RAM

        // 运行时堆栈信息
        uint32_t freertos_heap_used;  // 已使用的堆空间
        uint32_t freertos_heap_free;  // 剩余堆空间
        uint32_t libc_heap_watermark; // 最近可以分配的地址
        uint32_t stack_used;          // 已使用的栈空间（当前任务）
        uint32_t stack_free;          // 剩余栈空间（当前任务）

        // Flash使用情况
        uint32_t text_size;   // 代码段大小
        uint32_t rodata_size; // 只读数据段大小
        uint32_t used_flash;  // 已使用Flash
        uint32_t free_flash;  // 剩余Flash
    } memory_info_t;

    // 任务栈信息结构
    typedef struct
    {
        char task_name[16];  // 任务名称
        uint32_t stack_size; // 栈总大小
        uint32_t stack_used; // 已使用栈大小
        uint32_t stack_free; // 剩余栈大小
        float usage_percent; // 使用百分比
    } task_stack_info_t;

    /**
     * @brief 获取系统内存信息
     * @param info 内存信息结构指针
     * @return 0:成功 -1:失败
     */
    int memory_get_info(memory_info_t *info);

    /**
     * @brief 打印内存使用报告
     */
    void memory_print_report(void);

    /**
     * @brief 获取堆使用情况
     * @param total_size 总堆大小指针
     * @param used_size 已使用堆大小指针
     * @param free_size 剩余堆大小指针
     * @return 0:成功 -1:失败
     */
    int memory_get_heap_usage(uint32_t *total_size, uint32_t *used_size, uint32_t *free_size);

    /**
     * @brief 获取当前任务栈使用情况
     * @param info 栈信息结构指针
     * @return 0:成功 -1:失败
     */
    int memory_get_current_task_stack(task_stack_info_t *info);

    /**
     * @brief 获取所有任务的栈使用情况
     * @param info_array 栈信息数组
     * @param max_tasks 最大任务数
     * @return 实际任务数量
     */
    int memory_get_all_tasks_stack(task_stack_info_t *info_array, int max_tasks);

    /**
     * @brief 检查内存健康状况
     * @return true:健康 false:有问题
     */
    bool memory_health_check(void);

    /**
     * @brief 获取全局变量占用空间
     * @param data_size .data段大小指针
     * @param bss_size .bss段大小指针
     * @return 0:成功 -1:失败
     */
    int memory_get_global_usage(uint32_t *data_size, uint32_t *bss_size);

    /**
     * @brief 内存碎片化分析
     * @param largest_free 最大连续空闲块大小指针
     * @param fragmentation_percent 碎片化百分比指针
     * @return 0:成功 -1:失败
     */
    int memory_fragmentation_analysis(uint32_t *largest_free, float *fragmentation_percent);

#ifdef __cplusplus
}
#endif