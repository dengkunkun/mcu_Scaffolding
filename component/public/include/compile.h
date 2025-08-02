#pragma once
#include <stdint.h>
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

/* ==================== 内存段管理宏定义 ==================== */

/**
 * @brief 将函数或变量放置到特定内存段
 * @param section_name 段名称
 *
 * 支持的内存段类型：
 * - RAMFUNC: 将函数放到RAM中执行（提高执行速度）
 * - FASTDATA: 将数据放到快速访问的RAM区域
 * - BACKUP: 将数据放到备份区域（如果支持）
 * - SHARED: 将数据放到共享内存段
 * - CRITICAL: 将关键代码/数据放到特定段
 */
#define SECTION(section_name) __attribute__((section(section_name)))

/* 预定义的常用内存段宏 */
#define RAMFUNC SECTION(".ramfunc")             /* RAM中执行的函数 */
#define FASTDATA SECTION(".fastdata")           /* 快速访问数据段 */
#define BACKUP_DATA SECTION(".backup")          /* 备份数据段 */
#define SHARED_DATA SECTION(".shared")          /* 共享数据段 */
#define CRITICAL_CODE SECTION(".critical_code") /* 关键代码段 */
#define CRITICAL_DATA SECTION(".critical_data") /* 关键数据段 */
#define BOOTLOADER_DATA SECTION(".bootloader")  /* 引导加载器数据 */
#define CONFIG_DATA SECTION(".config")          /* 配置数据段 */

/* 对齐宏 */
#define ALIGNED(n) __attribute__((aligned(n))) /* n字节对齐 */
#define PACKED __attribute__((packed))         /* 紧凑结构体 */

/* 组合宏：常用的内存段+对齐 */
#define RAMFUNC_ALIGNED(n) RAMFUNC ALIGNED(n)             /* RAM函数+对齐 */
#define FASTDATA_ALIGNED(n) FASTDATA ALIGNED(n)           /* 快速数据+对齐 */
#define CRITICAL_ALIGNED(n) CRITICAL_CODE ALIGNED(n)      /* 关键代码+对齐 */
#define CRITICAL_DATA_ALIGNED(n) CRITICAL_DATA ALIGNED(n) /* 关键数据+对齐 */
#define SHARED_DATA_ALIGNED(n) SHARED_DATA ALIGNED(n)     /* 共享数据+对齐 */
#define BACKUP_DATA_ALIGNED(n) BACKUP_DATA ALIGNED(n)     /* 备份数据+对齐 */
#define CONFIG_DATA_ALIGNED(n) CONFIG_DATA ALIGNED(n)     /* 配置数据+对齐 */

/* 初始化优先级宏（用于构造函数排序）*/
#define INIT_PRIORITY(n) __attribute__((constructor(n)))
#define EXIT_PRIORITY(n) __attribute__((destructor(n)))

/* 内存段信息获取宏 */
#define DECLARE_SECTION_SYMBOLS(section_name)   \
    extern uint32_t __##section_name##_start__; \
    extern uint32_t __##section_name##_end__;   \
    extern uint32_t __##section_name##_size__

/* 获取段信息的辅助宏 */
#define SECTION_START(section_name) ((uint32_t)&__##section_name##_start__)
#define SECTION_END(section_name) ((uint32_t)&__##section_name##_end__)
#define SECTION_SIZE(section_name) ((uint32_t)&__##section_name##_size__)

/* 声明常用段的符号 */
DECLARE_SECTION_SYMBOLS(ramfunc);
DECLARE_SECTION_SYMBOLS(fastdata);
DECLARE_SECTION_SYMBOLS(backup);
DECLARE_SECTION_SYMBOLS(shared);
DECLARE_SECTION_SYMBOLS(critical_code);
DECLARE_SECTION_SYMBOLS(critical_data);
DECLARE_SECTION_SYMBOLS(bootloader);
DECLARE_SECTION_SYMBOLS(config);

/* 使用示例注释：
 *
 * // 将函数放到RAM中执行（提高性能）
 * RAMFUNC void fast_calculation(void) { ... }
 *
 * // 将数据放到快速访问段
 * FASTDATA uint32_t critical_buffer[1024];
 *
 * // 关键代码段（可用于加密、安全相关）
 * CRITICAL_CODE void security_function(void) { ... }
 *
 * // 配置数据（可用于参数存储）
 * CONFIG_DATA const config_t default_config = { ... };
 *
 * // 获取段信息
 * uint32_t ramfunc_start = SECTION_START(ramfunc);
 * uint32_t ramfunc_size = SECTION_SIZE(ramfunc);
 */

/* ==================== 性能测试宏定义 ==================== */

#include "hal.h"

/**
 * @brief DWT（Data Watchpoint and Trace）初始化
 * @note 必须在使用性能测试宏前调用一次
 */
static inline void dwt_init(void)
{
    // 启用DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // 清零周期计数器
    DWT->CYCCNT = 0;

    // 启用周期计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief 获取当前CPU时钟周期数
 * @return 当前周期计数
 */
static inline uint32_t dwt_get_cycles(void)
{
    return DWT->CYCCNT;
}

/**
 * @brief 获取系统时钟频率
 * @return 系统时钟频率（Hz）
 */
static inline uint32_t get_system_clock_freq(void)
{
    return HAL_RCC_GetSysClockFreq();
}

/**
 * @brief 将时钟周期转换为纳秒
 * @param cycles 时钟周期数
 * @param freq_hz 系统时钟频率（Hz）
 * @return 时间（纳秒）
 */
static inline uint64_t cycles_to_ns(uint32_t cycles, uint32_t freq_hz)
{
    return ((uint64_t)cycles * 1000000000ULL) / freq_hz;
}

/**
 * @brief 性能测试宏 - 测试代码块执行时间
 * @param code_block 要测试的代码块
 *
 * 使用示例：
 * BENCHMARK_CODE({
 *     // 你的代码
 *     for(int i = 0; i < 1000; i++) {
 *         // 一些计算
 *     }
 * });
 */
#define BENCHMARK_CODE(code_block)                                       \
    do                                                                   \
    {                                                                    \
        uint32_t start_cycles, end_cycles, elapsed_cycles;               \
        uint32_t sys_freq;                                               \
        uint64_t elapsed_ns;                                             \
        uint32_t elapsed_ns_low, elapsed_ms;                             \
                                                                         \
        sys_freq = get_system_clock_freq();                              \
        start_cycles = dwt_get_cycles();                                 \
                                                                         \
        code_block                                                       \
                                                                         \
            end_cycles = dwt_get_cycles();                               \
        elapsed_cycles = end_cycles - start_cycles;                      \
        elapsed_ns = cycles_to_ns(elapsed_cycles, sys_freq);             \
        elapsed_ns_low = (uint32_t)elapsed_ns;                           \
        elapsed_ms = elapsed_ns_low / 1000000;                           \
                                                                         \
        printf("[BENCHMARK] Cycles: %lu, Time: %lu ns (%lu.%03lu ms)\n", \
               (unsigned long)elapsed_cycles,                            \
               (unsigned long)elapsed_ns_low,                            \
               (unsigned long)elapsed_ms,                                \
               (unsigned long)(elapsed_ns_low % 1000000) / 1000);        \
    } while (0)

/**
 * @brief 性能测试宏 - 测试函数执行时间
 * @param func_call 要测试的函数调用
 *
 * 使用示例：
 * BENCHMARK_FUNCTION(my_function(param1, param2));
 */
#define BENCHMARK_FUNCTION(func_call)                                         \
    do                                                                        \
    {                                                                         \
        uint32_t start_cycles, end_cycles, elapsed_cycles;                    \
        uint32_t sys_freq;                                                    \
        uint64_t elapsed_ns;                                                  \
        uint32_t elapsed_ns_low, elapsed_ms;                                  \
                                                                              \
        sys_freq = get_system_clock_freq();                                   \
        start_cycles = dwt_get_cycles();                                      \
                                                                              \
        func_call;                                                            \
                                                                              \
        end_cycles = dwt_get_cycles();                                        \
        elapsed_cycles = end_cycles - start_cycles;                           \
        elapsed_ns = cycles_to_ns(elapsed_cycles, sys_freq);                  \
        elapsed_ns_low = (uint32_t)elapsed_ns;                                \
        elapsed_ms = elapsed_ns_low / 1000000;                                \
                                                                              \
        printf("[BENCHMARK] %s - Cycles: %lu, Time: %lu ns (%lu.%03lu ms)\n", \
               #func_call,                                                    \
               (unsigned long)elapsed_cycles,                                 \
               (unsigned long)elapsed_ns_low,                                 \
               (unsigned long)elapsed_ms,                                     \
               (unsigned long)(elapsed_ns_low % 1000000) / 1000);             \
    } while (0)

/**
 * @brief 带标签的性能测试宏
 * @param label 测试标签
 * @param code_block 要测试的代码块
 *
 * 使用示例：
 * BENCHMARK_LABELED("Matrix Multiplication", {
 *     matrix_multiply(a, b, result);
 * });
 */
#define BENCHMARK_LABELED(label, code_block)                                  \
    do                                                                        \
    {                                                                         \
        uint32_t start_cycles, end_cycles, elapsed_cycles;                    \
        uint32_t sys_freq;                                                    \
        uint64_t elapsed_ns;                                                  \
        uint32_t elapsed_ns_low, elapsed_ms;                                  \
                                                                              \
        sys_freq = get_system_clock_freq();                                   \
        start_cycles = dwt_get_cycles();                                      \
                                                                              \
        code_block                                                            \
                                                                              \
            end_cycles = dwt_get_cycles();                                    \
        elapsed_cycles = end_cycles - start_cycles;                           \
        elapsed_ns = cycles_to_ns(elapsed_cycles, sys_freq);                  \
        elapsed_ns_low = (uint32_t)elapsed_ns;                                \
        elapsed_ms = elapsed_ns_low / 1000000;                                \
                                                                              \
        printf("[BENCHMARK] %s - Cycles: %lu, Time: %lu ns (%lu.%03lu ms)\n", \
               label,                                                         \
               (unsigned long)elapsed_cycles,                                 \
               (unsigned long)elapsed_ns_low,                                 \
               (unsigned long)elapsed_ms,                                     \
               (unsigned long)(elapsed_ns_low % 1000000) / 1000);             \
    } while (0)

/**
 * @brief 多次运行平均性能测试宏
 * @param iterations 运行次数
 * @param code_block 要测试的代码块
 *
 * 使用示例：
 * BENCHMARK_AVERAGE(100, {
 *     quick_sort(array, 0, array_size-1);
 * });
 */
#define BENCHMARK_AVERAGE(iterations, code_block)                                               \
    do                                                                                          \
    {                                                                                           \
        uint32_t start_cycles, end_cycles;                                                      \
        uint64_t total_cycles = 0;                                                              \
        uint32_t sys_freq;                                                                      \
        uint32_t avg_cycles, avg_ns, avg_ms;                                                    \
                                                                                                \
        sys_freq = get_system_clock_freq();                                                     \
                                                                                                \
        for (int _i = 0; _i < iterations; _i++)                                                 \
        {                                                                                       \
            start_cycles = dwt_get_cycles();                                                    \
            code_block                                                                          \
                end_cycles = dwt_get_cycles();                                                  \
            total_cycles += (end_cycles - start_cycles);                                        \
        }                                                                                       \
                                                                                                \
        avg_cycles = (uint32_t)(total_cycles / iterations);                                     \
        avg_ns = (uint32_t)cycles_to_ns(avg_cycles, sys_freq);                                  \
        avg_ms = avg_ns / 1000000;                                                              \
                                                                                                \
        printf("[BENCHMARK] Average over %d runs - Cycles: %lu, Time: %lu ns (%lu.%03lu ms)\n", \
               iterations,                                                                      \
               (unsigned long)avg_cycles,                                                       \
               (unsigned long)avg_ns,                                                           \
               (unsigned long)avg_ms,                                                           \
               (unsigned long)(avg_ns % 1000000) / 1000);                                       \
    } while (0)