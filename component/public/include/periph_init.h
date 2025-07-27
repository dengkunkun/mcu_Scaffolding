/**
 * @file periph_init.h
 * @brief 外设自动初始化框架头文件
 * 
 * 该框架提供了一个通用的外设自动初始化机制，允许外设驱动模块
 * 通过宏定义自动注册初始化函数，系统启动时会按优先级顺序执行
 * 所有注册的初始化函数。
 * 
 * 主要特性：
 * - 基于链接器段的自动注册机制
 * - 支持优先级排序
 * - 详细的错误报告和统计信息
 * - 支持按名称或优先级范围初始化
 * - 内置调试和验证功能
 * 
 * @version 1.0
 * @date 2025-01-27
 */

#ifndef __PERIPH_INIT_H__
#define __PERIPH_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * 头文件包含
 * ============================================================================= */

#include <stdint.h>
#include <stddef.h>

/* =============================================================================
 * 宏定义
 * ============================================================================= */

/**
 * @brief 外设初始化函数注册宏
 * 
 * 使用此宏将外设初始化函数注册到系统中。函数会被自动放置到
 * .periph_init 段中，并在系统启动时按优先级顺序执行。
 * 
 * @param _name 外设名称（字符串）
 * @param _priority 初始化优先级（0-65535，数值越小优先级越高）
 * @param _func 初始化函数指针
 * @param _param 传递给初始化函数的参数
 * 
 * @note 优先级建议：
 *       - 0-99:   系统级外设（时钟、电源管理等）
 *       - 100-199: 基础外设（GPIO、UART等）
 *       - 200-299: 中级外设（SPI、I2C等）
 *       - 300-399: 高级外设（USB、网络等）
 *       - 400+:    应用级外设
 * 
 * @example
 * ```c
 * // 注册UART初始化函数，优先级为100
 * PERIPH_INIT_REGISTER("UART1", 100, uart1_init, &uart1_config);
 * 
 * // 注册GPIO初始化函数，优先级为50
 * PERIPH_INIT_REGISTER("GPIO", 50, gpio_init, NULL);
 * ```
 */
#define PERIPH_INIT_REGISTER(_name, _priority, _func, _param) \
    static const periph_init_desc_t __periph_init_##_func \
    __attribute__((used, section(".periph_init." #_priority))) = { \
        .name = _name, \
        .priority = _priority, \
        .init_func = _func, \
        .param = _param \
    }

/**
 * @brief 创建外设初始化函数的辅助宏
 * 
 * 该宏用于创建标准的外设初始化函数签名，确保函数签名的一致性。
 * 
 * @param _func_name 函数名称
 * 
 * @example
 * ```c
 * PERIPH_INIT_FUNCTION(uart1_init)
 * {
 *     UART_InitTypeDef *config = (UART_InitTypeDef*)param;
 *     // 初始化代码...
 *     return 0; // 成功返回0，失败返回非0
 * }
 * ```
 */
#define PERIPH_INIT_FUNCTION(_func_name) \
    int _func_name(void *param)

/* =============================================================================
 * 类型定义
 * ============================================================================= */

/**
 * @brief 外设初始化函数类型定义
 * 
 * 所有外设初始化函数必须符合此签名。
 * 
 * @param param 初始化参数指针（可以为NULL）
 * @return int 返回值：0表示成功，非0表示失败
 */
typedef int (*periph_init_func_t)(void *param);

/**
 * @brief 外设初始化描述符
 * 
 * 该结构体描述了一个外设的初始化信息，包括名称、优先级、
 * 初始化函数和参数。
 */
typedef struct {
    const char *name;           /**< 外设名称（用于调试和错误报告） */
    uint16_t priority;          /**< 初始化优先级（0-65535，越小优先级越高） */
    periph_init_func_t init_func; /**< 初始化函数指针 */
    void *param;                /**< 传递给初始化函数的参数 */
} periph_init_desc_t;

/**
 * @brief 外设初始化结果枚举
 * 
 * 定义了外设初始化过程可能的结果状态。
 */
typedef enum {
    PERIPH_INIT_OK = 0,         /**< 所有外设初始化成功 */
    PERIPH_INIT_PARTIAL_FAIL,   /**< 部分外设初始化失败 */
    PERIPH_INIT_ALL_FAIL,       /**< 所有外设初始化失败 */
    PERIPH_INIT_NO_DEVICES      /**< 没有找到任何外设 */
} periph_init_result_t;

/**
 * @brief 外设初始化统计信息
 * 
 * 包含了外设初始化过程的详细统计信息。
 */
typedef struct {
    uint32_t total_count;       /**< 总的外设数量 */
    uint32_t success_count;     /**< 成功初始化的外设数量 */
    uint32_t failed_count;      /**< 初始化失败的外设数量 */
    const char *first_failed_name; /**< 第一个失败的外设名称 */
} periph_init_stats_t;

/* =============================================================================
 * 核心API函数声明
 * ============================================================================== */

/**
 * @brief 初始化所有注册的外设
 * 
 * 按优先级顺序执行所有注册的外设初始化函数。
 * 
 * @param stats 输出参数，用于获取初始化统计信息（可为NULL）
 * @return periph_init_result_t 初始化结果
 * 
 * @example
 * ```c
 * periph_init_stats_t stats;
 * periph_init_result_t result = periph_init_all(&stats);
 * 
 * if (result != PERIPH_INIT_OK) {
 *     printf("Peripheral initialization failed!\n");
 *     periph_init_print_stats(&stats);
 * }
 * ```
 */
periph_init_result_t periph_init_all(periph_init_stats_t *stats);

/**
 * @brief 根据名称初始化指定的外设
 * 
 * @param name 外设名称
 * @return int 0表示成功，负数表示失败：
 *         -1: 参数错误
 *         -2: 初始化函数为空
 *         -3: 未找到指定名称的外设
 *         其他: 初始化函数返回的错误码
 * 
 * @example
 * ```c
 * int result = periph_init_by_name("UART1");
 * if (result != 0) {
 *     printf("UART1 initialization failed: %d\n", result);
 * }
 * ```
 */
int periph_init_by_name(const char *name);

/**
 * @brief 根据优先级范围初始化外设
 * 
 * 只初始化优先级在指定范围内的外设。
 * 
 * @param min_priority 最小优先级（包含）
 * @param max_priority 最大优先级（包含）
 * @param stats 输出参数，用于获取初始化统计信息（可为NULL）
 * @return periph_init_result_t 初始化结果
 * 
 * @example
 * ```c
 * // 只初始化基础外设（优先级100-199）
 * periph_init_stats_t stats;
 * periph_init_result_t result = periph_init_by_priority_range(100, 199, &stats);
 * ```
 */
periph_init_result_t periph_init_by_priority_range(uint16_t min_priority, 
                                                  uint16_t max_priority,
                                                  periph_init_stats_t *stats);

/**
 * @brief 获取所有外设描述符
 * 
 * 返回指向外设描述符数组的指针，用于高级操作和调试。
 * 
 * @param count 输出参数，用于返回描述符数量
 * @return const periph_init_desc_t* 描述符数组指针，失败返回NULL
 * 
 * @note 返回的指针指向只读数据，不应修改
 * 
 * @example
 * ```c
 * uint32_t count;
 * const periph_init_desc_t *descriptors = periph_init_get_descriptors(&count);
 * 
 * for (uint32_t i = 0; i < count; i++) {
 *     printf("Device: %s, Priority: %u\n", 
 *            descriptors[i].name, descriptors[i].priority);
 * }
 * ```
 */
const periph_init_desc_t* periph_init_get_descriptors(uint32_t *count);

/* =============================================================================
 * 辅助函数声明
 * ============================================================================== */

/**
 * @brief 打印外设初始化统计信息
 * 
 * 以友好的格式打印外设初始化的统计信息。
 * 
 * @param stats 统计信息指针
 * 
 * @example
 * ```c
 * periph_init_stats_t stats;
 * periph_init_all(&stats);
 * periph_init_print_stats(&stats);
 * ```
 */
void periph_init_print_stats(const periph_init_stats_t *stats);

/**
 * @brief 计算初始化成功率
 * 
 * @param stats 统计信息指针
 * @return float 成功率百分比（0.0-100.0）
 */
static inline float periph_init_success_rate(const periph_init_stats_t *stats)
{
    if (stats == NULL || stats->total_count == 0) {
        return 0.0f;
    }
    
    return (float)stats->success_count / stats->total_count * 100.0f;
}

/**
 * @brief 检查是否有初始化失败的外设
 * 
 * @param stats 统计信息指针
 * @return int 1表示有失败，0表示无失败
 */
static inline int periph_init_has_failures(const periph_init_stats_t *stats)
{
    return (stats != NULL) ? (stats->failed_count > 0 ? 1 : 0) : 0;
}

/* =============================================================================
 * 调试和诊断函数声明
 * ============================================================================== */

/**
 * @brief 打印所有注册的外设信息（调试用）
 * 
 * 显示所有注册的外设的详细信息，包括名称、优先级、函数地址等。
 * 主要用于调试和验证注册是否正确。
 */
void periph_init_debug_print_all(void);

/**
 * @brief 验证外设初始化框架的完整性
 * 
 * 检查框架的配置是否正确，包括链接器脚本配置、描述符完整性等。
 * 
 * @return int 0表示验证成功，非0表示发现问题
 */
int periph_init_verify_integrity(void);

/* =============================================================================
 * 常用优先级定义
 * ============================================================================== */

/**
 * @brief 预定义的优先级常量
 * 
 * 提供了一些常用的优先级数值，方便使用。
 */
#define PERIPH_INIT_PRIORITY_CRITICAL    0    /**< 关键优先级（最高） */
#define PERIPH_INIT_PRIORITY_SYSTEM      50   /**< 系统级优先级 */
#define PERIPH_INIT_PRIORITY_BASIC       100  /**< 基础外设优先级 */
#define PERIPH_INIT_PRIORITY_MIDDLE      200  /**< 中级外设优先级 */
#define PERIPH_INIT_PRIORITY_ADVANCED    300  /**< 高级外设优先级 */
#define PERIPH_INIT_PRIORITY_APPLICATION 400  /**< 应用级优先级 */
#define PERIPH_INIT_PRIORITY_LOW         500  /**< 低优先级 */
#define PERIPH_INIT_PRIORITY_LOWEST      1000 /**< 最低优先级 */

/* =============================================================================
 * 错误代码定义
 * ============================================================================== */

/**
 * @brief 预定义的错误代码
 */
#define PERIPH_INIT_ERR_OK              0    /**< 成功 */
#define PERIPH_INIT_ERR_PARAM           -1   /**< 参数错误 */
#define PERIPH_INIT_ERR_NULL_FUNC       -2   /**< 函数指针为空 */
#define PERIPH_INIT_ERR_NOT_FOUND       -3   /**< 未找到指定外设 */
#define PERIPH_INIT_ERR_HARDWARE        -10  /**< 硬件错误 */
#define PERIPH_INIT_ERR_TIMEOUT         -11  /**< 超时错误 */
#define PERIPH_INIT_ERR_BUSY            -12  /**< 设备忙 */
#define PERIPH_INIT_ERR_NOT_SUPPORTED   -13  /**< 不支持的操作 */

#ifdef __cplusplus
}
#endif

#endif /* __PERIPH_INIT_H__ */