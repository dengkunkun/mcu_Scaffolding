#include "memory_monitor.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"

// 链接器符号 - 由链接脚本定义
extern uint32_t _sdata;  // .data段起始地址
extern uint32_t _edata;  // .data段结束地址
extern uint32_t _sbss;   // .bss段起始地址
extern uint32_t _ebss;   // .bss段结束地址
extern uint32_t _estack; // 栈顶地址
extern uint32_t end;     // 从调试器中可以直接访问，&end,不是end
extern uint32_t _end;
extern uint32_t _Min_Heap_Size;  // libc堆大小；&_Min_Heap_Size=链接器脚本中的值，_Min_Heap_Size是一个不可访问的值
extern uint32_t _Min_Stack_Size; // 栈大小

// STM32F103ZET6 内存配置
#define TOTAL_RAM_SIZE (64 * 1024)    // 64KB RAM
#define TOTAL_FLASH_SIZE (512 * 1024) // 512KB Flash
#define RAM_START_ADDR 0x20000000
#define RAM_END_ADDR (RAM_START_ADDR + TOTAL_RAM_SIZE)

int memory_get_global_usage(uint32_t *data_size, uint32_t *bss_size)
{
    if (!data_size || !bss_size)
    {
        return -1;
    }

    // 计算.data段大小（初始化的全局变量）
    *data_size = (uint32_t)&_edata - (uint32_t)&_sdata;

    // 计算.bss段大小（未初始化的全局变量）
    *bss_size = (uint32_t)&_ebss - (uint32_t)&_sbss;

    return 0;
}

int memory_get_heap_usage(uint32_t *total_size, uint32_t *used_size, uint32_t *free_size)
{
    if (!total_size || !used_size || !free_size)
    {
        return -1;
    }

    // FreeRTOS堆管理信息
    *free_size = xPortGetFreeHeapSize();
    *total_size = configTOTAL_HEAP_SIZE;
    *used_size = *total_size - *free_size;

    return 0;
}

int memory_get_current_task_stack(task_stack_info_t *info)
{
    if (!info)
    {
        return -1;
    }

    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    if (!current_task)
    {
        return -1;
    }

    // 获取任务名称
    const char *task_name = pcTaskGetName(current_task);
    strncpy(info->task_name, task_name ? task_name : "Unknown", sizeof(info->task_name) - 1);
    info->task_name[sizeof(info->task_name) - 1] = '\0';

    // 获取栈使用情况
    info->stack_free = uxTaskGetStackHighWaterMark(current_task) * sizeof(StackType_t);

    // 注意：FreeRTOS无法直接获取栈总大小，需要在任务创建时记录
    // 这里使用估算方法
    info->stack_size = info->stack_free + 512; // 估算值
    info->stack_used = info->stack_size - info->stack_free;
    info->usage_percent = ((float)info->stack_used / info->stack_size) * 100.0f;

    return 0;
}

int memory_get_all_tasks_stack(task_stack_info_t *info_array, int max_tasks)
{
    if (!info_array || max_tasks <= 0)
    {
        return -1;
    }

    TaskStatus_t *task_status_array;
    UBaseType_t task_count;
    int info_count = 0;

    // 获取任务数量
    task_count = uxTaskGetNumberOfTasks();
    if (task_count == 0)
    {
        return 0;
    }

    // 分配临时数组
    task_status_array = pvPortMalloc(task_count * sizeof(TaskStatus_t));
    if (!task_status_array)
    {
        return -1;
    }

    // 获取所有任务状态
    task_count = uxTaskGetSystemState(task_status_array, task_count, NULL);

    // 填充信息数组
    for (UBaseType_t i = 0; i < task_count && info_count < max_tasks; i++)
    {
        task_stack_info_t *info = &info_array[info_count];

        // 任务名称
        strncpy(info->task_name, task_status_array[i].pcTaskName, sizeof(info->task_name) - 1);
        info->task_name[sizeof(info->task_name) - 1] = '\0';

        // 栈信息
        info->stack_free = task_status_array[i].usStackHighWaterMark * sizeof(StackType_t);
        info->stack_size = info->stack_free + 512; // 没有获取栈大小的接口，只能在启动线程时自己记录
        info->stack_used = info->stack_size - info->stack_free;
        info->usage_percent = ((float)info->stack_used / info->stack_size) * 100.0f;

        info_count++;
    }

    vPortFree(task_status_array);
    return info_count;
}

int memory_get_info(memory_info_t *info)
{
    if (!info)
    {
        return -1;
    }

    memset(info, 0, sizeof(memory_info_t));

    // 基本内存信息
    info->total_ram = TOTAL_RAM_SIZE;
    info->total_flash = TOTAL_FLASH_SIZE;

    // 获取全局变量使用情况
    memory_get_global_usage(&info->data_size, &info->bss_size);

    // 获取堆使用情况
    memory_get_heap_usage(&info->freertos_heap_size, &info->freertos_heap_used, &info->freertos_heap_free);

    // 计算剩余RAM（简化计算）
    uint32_t used_ram = info->data_size + info->bss_size + info->freertos_heap_used;
    info->free_ram = info->total_ram - used_ram;

    // 获取当前任务栈信息
    task_stack_info_t stack_info;
    if (memory_get_current_task_stack(&stack_info) == 0)
    {
        info->stack_used = stack_info.stack_used;
        info->stack_free = stack_info.stack_free;
        info->stack_size = stack_info.stack_size;
    }

    // Flash使用估算（需要从map文件或符号获取准确值）
    info->text_size = 36384;  // 从编译结果获取
    info->rodata_size = 2128; // 从编译结果获取
    info->used_flash = info->text_size + info->rodata_size + info->data_size;
    info->free_flash = info->total_flash - info->used_flash;

    return 0;
}

void memory_print_report(void)
{
    memory_info_t info;
    if (memory_get_info(&info) != 0)
    {
        logi("Failed to get memory info");
        return;
    }

    logi("\n=== Memory Usage Report ===\n");
    int count = 4;
    void *p = NULL, *start = NULL, *end = NULL;
    while ((p = malloc(count)) != NULL)
    {
        if (!start)
        {
            start = p;
        }
        end = p;
        count += 128;
        free(p);
    }
    logi("malloc max available:%d start:%p end:%p", count, start, end);
    logi("end from linker script:%p", &end);
    count = 4;
    start = end = NULL;
    while ((p = pvPortMalloc(count)) != NULL)
    {
        if (!start)
        {
            start = p;
        }
        end = p;
        count += 128;
        vPortFree(p);
    }
    logi("pvPortMalloc max available:%d start:%p end:%p", count, start, end);
    // RAM使用情况
    logi("RAM Usage:\n");
    logi("  Total RAM:        %lu bytes (%.1f KB)",
         info.total_ram, info.total_ram / 1024.0f);
    logi("  Global vars(.data):\t\t%lu bytes", info.data_size);
    logi("  Global vars(.bss):\t\t%lu bytes", info.bss_size);
    logi("  freertos Heap used:\t\t%lu bytes", info.freertos_heap_used);
    logi("  freertos sHeap free:\t\t%lu bytes", info.freertos_heap_free);
    logi("  Stack used:\t\t%lu bytes (current task)", info.stack_used);
    logi("  Free RAM:\t\t%lu bytes (%.1f KB)",
         info.free_ram, info.free_ram / 1024.0f);
    logi("  RAM utilization:  %.1f%%",
         ((float)(info.total_ram - info.free_ram) / info.total_ram) * 100.0f);

    // Flash使用情况
    logi("\nFlash Usage:\n");
    logi("  Total Flash:      %lu bytes (%.1f KB)",
         info.total_flash, info.total_flash / 1024.0f);
    logi("  Code (.text):     %lu bytes", info.text_size);
    logi("  Constants (.rodata): %lu bytes", info.rodata_size);
    logi("  Init data:        %lu bytes", info.data_size);
    logi("  Used Flash:       %lu bytes (%.1f KB)",
         info.used_flash, info.used_flash / 1024.0f);
    logi("  Free Flash:       %lu bytes (%.1f KB)",
         info.free_flash, info.free_flash / 1024.0f);
    logi("  Flash utilization: %.1f%%",
         ((float)info.used_flash / info.total_flash) * 100.0f);

    // 任务栈使用情况
    logi("\nTask Stack Usage:\n");
    task_stack_info_t stack_info[10];
    int task_count = memory_get_all_tasks_stack(stack_info, 10);

    for (int i = 0; i < task_count; i++)
    {
        logi("  %-12s: %lu/%lu bytes (%.1f%% used)",
             stack_info[i].task_name,
             stack_info[i].stack_used,
             stack_info[i].stack_size,
             stack_info[i].usage_percent);
    }
}

bool memory_health_check(void)
{

    memory_info_t info;
    if (memory_get_info(&info) != 0)
    {
        return false;
    }

    // 检查RAM使用率
    float ram_usage = ((float)(info.total_ram - info.free_ram) / info.total_ram) * 100.0f;
    if (ram_usage > 90.0f)
    {
        logi("WARNING: RAM usage too high: %.1f%%", ram_usage);
        return false;
    }

    // 检查堆使用率
    float heap_usage = ((float)info.freertos_heap_used / info.freertos_heap_size) * 100.0f;
    if (heap_usage > 85.0f)
    {
        logi("WARNING: Heap usage too high: %.1f%%", heap_usage);
        return false;
    }

    // 检查最小剩余堆空间
    uint32_t min_free_heap = xPortGetMinimumEverFreeHeapSize();
    if (min_free_heap < 1024)
    { // 少于1KB
        logi("WARNING: Minimum free heap too low: %lu bytes", min_free_heap);
        return false;
    }

    // 检查任务栈使用情况
    task_stack_info_t stack_info[10];
    int task_count = memory_get_all_tasks_stack(stack_info, 10);

    for (int i = 0; i < task_count; i++)
    {
        if (stack_info[i].usage_percent > 80.0f)
        {
            logi("WARNING: Task '%s' stack usage too high: %.1f%%",
                 stack_info[i].task_name, stack_info[i].usage_percent);
            return false;
        }

        if (stack_info[i].stack_free < 128)
        { // 少于128字节
            logi("WARNING: Task '%s' stack free too low: %lu bytes",
                 stack_info[i].task_name, stack_info[i].stack_free);
            return false;
        }
    }

    return true;
}

int memory_fragmentation_analysis(uint32_t *largest_free, float *fragmentation_percent)
{
    if (!largest_free || !fragmentation_percent)
    {
        return -1;
    }

    // 简化的碎片化分析
    // 注意：FreeRTOS heap_4.c 不直接提供碎片化信息

    uint32_t total_free = xPortGetFreeHeapSize();

    // 尝试分配一个大块来估算最大连续空闲块
    uint32_t test_size = total_free;
    void *test_ptr = NULL;

    // 二分查找最大可分配块
    uint32_t min_size = 0;
    uint32_t max_size = total_free;

    while (min_size < max_size - 1)
    {
        test_size = (min_size + max_size) / 2;
        test_ptr = pvPortMalloc(test_size);

        if (test_ptr)
        {
            vPortFree(test_ptr);
            min_size = test_size;
        }
        else
        {
            max_size = test_size;
        }
    }

    *largest_free = min_size;

    if (total_free > 0)
    {
        *fragmentation_percent = (1.0f - ((float)*largest_free / total_free)) * 100.0f;
    }
    else
    {
        *fragmentation_percent = 0.0f;
    }

    return 0;
}