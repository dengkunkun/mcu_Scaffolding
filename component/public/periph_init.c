/**
 * @file periph_init.c
 * @brief 外设自动初始化框架实现文件
 *
 * @version 1.0
 * @date 2025-01-27
 */

#include "periph_init.h"
#include <string.h>
#include <stdio.h>

/* =============================================================================
 * 链接器段符号声明
 * ============================================================================== */

/**
 * @brief 外设初始化段的起始地址
 *
 * 这个符号由链接器脚本定义，指向.periph_init段的开始位置
 */
extern const periph_init_desc_t __periph_init_start;

/**
 * @brief 外设初始化段的结束地址
 *
 * 这个符号由链接器脚本定义，指向.periph_init段的结束位置
 */
extern const periph_init_desc_t __periph_init_end;

/* =============================================================================
 * 内部辅助函数
 * ============================================================================== */

/**
 * @brief 获取外设初始化描述符数组的起始地址和数量
 *
 * @param count 输出参数，用于返回描述符数量
 * @return const periph_init_desc_t* 描述符数组的起始地址
 */
static const periph_init_desc_t *get_periph_descriptors(uint32_t *count)
{
    if (count == NULL)
    {
        return NULL;
    }

    // 计算描述符数量
    const periph_init_desc_t *start = &__periph_init_start;
    const periph_init_desc_t *end = &__periph_init_end;

    // 计算数组长度
    *count = (uint32_t)(end - start);

    return start;
}

/**
 * @brief 安全的字符串比较函数
 *
 * @param str1 第一个字符串
 * @param str2 第二个字符串
 * @return int 0表示相等，非0表示不相等
 */
static int safe_strcmp(const char *str1, const char *str2)
{
    if (str1 == NULL || str2 == NULL)
    {
        return (str1 == str2) ? 0 : 1;
    }
    return strcmp(str1, str2);
}

/**
 * @brief 执行单个外设的初始化
 *
 * @param desc 外设描述符指针
 * @return int 初始化结果，0表示成功，非0表示失败
 */
static int execute_single_init(const periph_init_desc_t *desc)
{
    if (desc == NULL || desc->init_func == NULL)
    {
        return -1;
    }

    // 执行初始化函数
    int result = desc->init_func(desc->param);
    printf("name:%s\tfunc:%p\tparam:%p\tprior:%d\tresult:%d\n",
           desc->name, desc->init_func, desc->param, desc->priority, result);
    return result;
}

/* =============================================================================
 * 核心API函数实现
 * ============================================================================== */

periph_init_result_t periph_init_all(periph_init_stats_t *stats)
{
    uint32_t count;
    const periph_init_desc_t *descriptors = get_periph_descriptors(&count);

    // 初始化统计信息
    periph_init_stats_t local_stats = {0};
    if (stats != NULL)
    {
        memset(stats, 0, sizeof(periph_init_stats_t));
    }

    // 如果没有找到任何描述符
    if (count == 0)
    {
        if (stats != NULL)
        {
            *stats = local_stats;
        }
        return PERIPH_INIT_NO_DEVICES;
    }

    local_stats.total_count = count;

    // 按顺序执行所有初始化函数
    for (uint32_t i = 0; i < count; i++)
    {
        const periph_init_desc_t *desc = &descriptors[i];
        // 执行初始化
        int result = execute_single_init(desc);

        if (result == 0)
        {
            local_stats.success_count++;
        }
        else
        {
            local_stats.failed_count++;

            // 记录第一个失败的外设名称
            if (local_stats.first_failed_name == NULL && desc->name != NULL)
            {
                local_stats.first_failed_name = desc->name;
            }
        }
    }

    // 复制统计信息
    if (stats != NULL)
    {
        *stats = local_stats;
    }

    // 确定返回结果
    if (local_stats.failed_count == 0)
    {
        return PERIPH_INIT_OK;
    }
    else if (local_stats.success_count > 0)
    {
        return PERIPH_INIT_PARTIAL_FAIL;
    }
    else
    {
        return PERIPH_INIT_ALL_FAIL;
    }
}

void periph_init_print_stats(const periph_init_stats_t *stats)
{
    if (stats == NULL)
    {
        printf("Error: stats parameter is NULL\n");
        return;
    }

    printf("=== Peripheral Initialization Statistics ===\n");
    printf("Total peripherals: %lu\n", stats->total_count);
    printf("Successfully initialized: %lu\n", stats->success_count);
    printf("Failed to initialize: %lu\n", stats->failed_count);

    if (stats->total_count > 0)
    {
        float success_rate = periph_init_success_rate(stats);
        printf("Success rate: %.1f%%\n", success_rate);
    }

    if (stats->failed_count > 0 && stats->first_failed_name != NULL)
    {
        printf("First failed peripheral: %s\n", stats->first_failed_name);
    }

    // 根据结果显示不同的状态
    if (stats->failed_count == 0)
    {
        printf("Status: ✓ ALL OK\n");
    }
    else if (stats->success_count > 0)
    {
        printf("Status: ⚠ PARTIAL FAILURE\n");
    }
    else
    {
        printf("Status: ✗ COMPLETE FAILURE\n");
    }

    printf("=============================================\n");
}

int periph_init_by_name(const char *name)
{
    if (name == NULL)
    {
        return -1; // 参数错误
    }

    uint32_t count;
    const periph_init_desc_t *descriptors = get_periph_descriptors(&count);

    // 查找指定名称的外设
    for (uint32_t i = 0; i < count; i++)
    {
        const periph_init_desc_t *desc = &descriptors[i];

        if (safe_strcmp(desc->name, name) == 0)
        {
            // 找到匹配的外设
            if (desc->init_func == NULL)
            {
                return -2; // 初始化函数为空
            }

            // 执行初始化
            int result = execute_single_init(desc);
            return result; // 返回初始化函数的结果
        }
    }

    return -3; // 未找到指定名称的外设
}

periph_init_result_t periph_init_by_priority_range(uint16_t min_priority,
                                                   uint16_t max_priority,
                                                   periph_init_stats_t *stats)
{
    uint32_t count;
    const periph_init_desc_t *descriptors = get_periph_descriptors(&count);

    // 初始化统计信息
    periph_init_stats_t local_stats = {0};
    if (stats != NULL)
    {
        memset(stats, 0, sizeof(periph_init_stats_t));
    }

    // 如果没有找到任何描述符
    if (count == 0)
    {
        if (stats != NULL)
        {
            *stats = local_stats;
        }
        return PERIPH_INIT_NO_DEVICES;
    }

    // 遍历所有描述符，只初始化优先级在指定范围内的
    for (uint32_t i = 0; i < count; i++)
    {
        const periph_init_desc_t *desc = &descriptors[i];

        // 检查优先级是否在范围内
        if (desc->priority >= min_priority && desc->priority <= max_priority)
        {
            local_stats.total_count++;

            // 执行初始化
            int result = execute_single_init(desc);

            if (result == 0)
            {
                local_stats.success_count++;
            }
            else
            {
                local_stats.failed_count++;

                // 记录第一个失败的外设名称
                if (local_stats.first_failed_name == NULL && desc->name != NULL)
                {
                    local_stats.first_failed_name = desc->name;
                }
            }
        }
    }

    // 复制统计信息
    if (stats != NULL)
    {
        *stats = local_stats;
    }

    // 如果没有找到符合条件的外设
    if (local_stats.total_count == 0)
    {
        return PERIPH_INIT_NO_DEVICES;
    }

    // 确定返回结果
    if (local_stats.failed_count == 0)
    {
        return PERIPH_INIT_OK;
    }
    else if (local_stats.success_count > 0)
    {
        return PERIPH_INIT_PARTIAL_FAIL;
    }
    else
    {
        return PERIPH_INIT_ALL_FAIL;
    }
}

const periph_init_desc_t *periph_init_get_descriptors(uint32_t *count)
{
    if (count == NULL)
    {
        return NULL;
    }

    return get_periph_descriptors(count);
}

/* =============================================================================
 * 调试和诊断函数
 * ============================================================================== */

/**
 * @brief 打印所有注册的外设信息（调试用）
 */
void periph_init_debug_print_all(void)
{
    uint32_t count;
    const periph_init_desc_t *descriptors = periph_init_get_descriptors(&count);

    printf("=== Registered Peripheral Devices ===\n");
    printf("Total count: %lu\n", count);
    printf("----------------------------------------\n");

    if (count == 0)
    {
        printf("No peripheral descriptors found!\n");
        printf("Check linker script configuration.\n");
    }
    else
    {
        for (uint32_t i = 0; i < count; i++)
        {
            const periph_init_desc_t *desc = &descriptors[i];

            printf("%2lu. Name: %-15s Priority: %4u  Func: %p  Param: %p\n",
                   i + 1,
                   desc->name ? desc->name : "NULL",
                   desc->priority,
                   (void *)desc->init_func,
                   desc->param);
        }
    }

    printf("=====================================\n");
}

/**
 * @brief 验证外设初始化框架的完整性
 *
 * @return int 0表示验证成功，非0表示发现问题
 */
int periph_init_verify_integrity(void)
{
    uint32_t count;
    const periph_init_desc_t *descriptors = periph_init_get_descriptors(&count);

    int issues = 0;

    printf("=== Peripheral Initialization Framework Verification ===\n");

    // 检查是否找到了描述符
    if (count == 0)
    {
        printf("❌ No peripheral descriptors found\n");
        printf("   Check linker script .periph_init section configuration\n");
        issues++;
    }
    else
    {
        printf("✓ Found %lu peripheral descriptors\n", count);
    }

    // 检查每个描述符的完整性
    for (uint32_t i = 0; i < count; i++)
    {
        const periph_init_desc_t *desc = &descriptors[i];

        // 检查初始化函数指针
        if (desc->init_func == NULL)
        {
            printf("❌ Descriptor %lu: init_func is NULL\n", i);
            issues++;
        }

        // 检查名称
        if (desc->name == NULL)
        {
            printf("⚠ Descriptor %lu: name is NULL (not critical)\n", i);
        }

        // 检查优先级顺序（应该是递增的，因为链接器已排序）
        if (i > 0 && desc->priority < descriptors[i - 1].priority)
        {
            printf("❌ Descriptor %lu: priority order violation (priority: %u, previous: %u)\n",
                   i, desc->priority, descriptors[i - 1].priority);
            printf("   Check linker script SORT(.periph_init.*) configuration\n");
            issues++;
        }
    }

    if (issues == 0)
    {
        printf("✓ Framework integrity verification passed\n");
    }
    else
    {
        printf("❌ Found %d issues during verification\n", issues);
    }

    printf("========================================================\n");

    return issues;
}

/* =============================================================================
 * 高级管理功能
 * ============================================================================== */

/**
 * @brief 统计各优先级范围的外设数量
 *
 * @param ranges 优先级范围数组
 * @param range_count 范围数量
 * @param counts 输出的计数数组
 */
void periph_init_count_by_priority_ranges(const struct {
    uint16_t min;
    uint16_t max;
    const char *name; } * ranges, size_t range_count, uint32_t *counts)
{
    if (ranges == NULL || counts == NULL)
    {
        return;
    }

    uint32_t total_count;
    const periph_init_desc_t *descriptors = periph_init_get_descriptors(&total_count);

    // 初始化计数数组
    for (size_t i = 0; i < range_count; i++)
    {
        counts[i] = 0;
    }

    // 统计每个范围的外设数量
    for (uint32_t i = 0; i < total_count; i++)
    {
        const periph_init_desc_t *desc = &descriptors[i];

        for (size_t j = 0; j < range_count; j++)
        {
            if (desc->priority >= ranges[j].min && desc->priority <= ranges[j].max)
            {
                counts[j]++;
                break; // 假设范围不重叠
            }
        }
    }
}