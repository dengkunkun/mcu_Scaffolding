#include "usart.h"
#include "gpio.h"
#include "main.h"
#include "cmsis_os2.h"
#include <stdio.h>

#include "log.h"
#include "shell.h"
#include "memory_monitor.h"
#include "periph_init.h"  // 添加外设初始化框架头文件
#include "version_info.h" // 添加版本信息头文件
#include "worker.h"
#include "compile.h"
#include "memory_sections.h" // 添加内存段管理头文件

int _write(int file, char *ptr, int len)
{
    (void)file; // Suppress unused parameter warning
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

#define LED_RED_TOGGLE() HAL_GPIO_TogglePin(led_red_GPIO_Port, led_red_Pin)
#define LED_BLUE_TOGGLE() HAL_GPIO_TogglePin(led_blue_GPIO_Port, led_blue_Pin)
#define LED_GREEN_TOGGLE() HAL_GPIO_TogglePin(led_green_GPIO_Port, led_green_Pin)
#define LED_RED_ON(on) HAL_GPIO_WritePin(led_red_GPIO_Port, led_red_Pin, !on)
#define LED_BLUE_ON(on) HAL_GPIO_WritePin(led_blue_GPIO_Port, led_blue_Pin, !on)
#define LED_GREEN_ON(on) HAL_GPIO_WritePin(led_green_GPIO_Port, led_green_Pin, !on)

void error_handler(const char *err_msg)
{
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    taskDISABLE_INTERRUPTS();
    // 简单的硬件延迟，不依赖FreeRTOS
    volatile uint32_t delay_count;

    while (1)
    {
        LED_RED_TOGGLE();

        // 使用简单的延迟循环，不调用FreeRTOS API
        for (delay_count = 0; delay_count < 1000000; delay_count++)
        {
            __NOP(); // 空操作，防止编译器优化
        }

        printf("%s\n", err_msg);

        // 再次延迟
        for (delay_count = 0; delay_count < 1000000; delay_count++)
        {
            __NOP();
        }
    }
}

/**
 * @brief 系统LED初始化函数
 */
static int system_leds_init(void *param)
{
    (void)param;     // 未使用参数
    LED_RED_ON(0);   // 红色LED关闭
    LED_BLUE_ON(0);  // 蓝色LED关闭
    LED_GREEN_ON(0); // 绿色LED关闭

    logi("System LEDs initialized");
    return 0;
}

/**
 * @brief 系统监控初始化函数
 */
static int system_monitor_init_func(void *param)
{
    (void)param;

    logi("System monitor subsystem initialized");
    return 0;
}

/**
 * @brief 应用层组件初始化函数
 */
static int app_components_init(void *param)
{
    (void)param;

    // 初始化应用层组件
    log_init();   // Initialize the logging system
    shell_init(); // Initialize the shell system

    logi("Application components initialized successfully");
    return 0;
}

// 注册外设初始化函数
PERIPH_INIT_REGISTER("SYSTEM_LEDS", 300, system_leds_init, NULL);
PERIPH_INIT_REGISTER("SYSTEM_MONITOR", 200, system_monitor_init_func, NULL);
PERIPH_INIT_REGISTER("APP_COMPONENTS", 100, app_components_init, NULL);

void system_monitor_work(void *arg)
{
    LED_BLUE_TOGGLE();
    logw("system_monitor_work");
}
FASTDATA static char *msg_fastdata1 = "01234567890";  // flash
FASTDATA static char msg_fastdata2[] = "01234567890"; // 移除const，使用数组而非指针,否则会被放到.rodata中
// 将FASTDATA变量移到函数外部
FASTDATA static char fastdata_msg[] = "01234567890";

// 一些测试函数
RAMFUNC void fast_math_operation(void)
{
    volatile uint32_t result = 0;
    for (int i = 0; i < 1000000; i++)
    {
        result += i * i;
    }
}

void slow_math_operation(void)
{
    volatile uint32_t result = 0;
    for (int i = 0; i < 1000000; i++)
    {
        result += i * i;
    }
}

void memory_copy_test(void)
{
    static uint8_t src[256];
    static uint8_t dst[256];

    // 填充源数据
    for (int i = 0; i < 256; i++)
    {
        src[i] = i & 0xFF;
    }

    // 复制数据
    for (int i = 0; i < 256; i++)
    {
        dst[i] = src[i];
    }
}

RAMFUNC void ram_func_test()
{
    logi("RAMFUNC ram_func_test addr:%p", ram_func_test);
    logi("FASTDATA msg:%p %s", fastdata_msg, fastdata_msg);

    // 初始化DWT - 只需要调用一次
    dwt_init();
    logi("DWT initialized, system clock: %lu Hz", get_system_clock_freq());

    // 测试单个函数执行时间
    BENCHMARK_FUNCTION(fast_math_operation());
    BENCHMARK_FUNCTION(slow_math_operation());

    // 测试代码块执行时间
    BENCHMARK_LABELED("Memory Copy Test", {
        memory_copy_test();
    });

    // 测试简单运算
    BENCHMARK_CODE({
        volatile uint32_t sum = 0;
        for (int i = 0; i < 100; i++)
        {
            sum += i;
        }
    });

    // 平均测试（运行10次取平均值）
    BENCHMARK_AVERAGE(10, {
        volatile uint32_t product = 1;
        for (int i = 1; i <= 20; i++)
        {
            product *= i;
        }
    });

    // 比较RAMFUNC和普通函数的性能差异
    logi("=== Performance Comparison: RAMFUNC vs Normal Function ===");
    BENCHMARK_LABELED("RAMFUNC (in RAM)", {
        fast_math_operation();
    });

    BENCHMARK_LABELED("Normal Function (in Flash)", {
        slow_math_operation();
    });
}
int app_main(void)

{
    // 首先初始化内存段 - 这必须在任何RAM函数被调用之前执行
    memory_sections_init();

    // 显示版本信息
    print_version_info();

    // 使用外设初始化框架自动初始化所有外设
    logi("Starting peripheral initialization...");

    periph_init_stats_t init_stats;
    periph_init_result_t init_result = periph_init_all(&init_stats);
    if (PERIPH_INIT_OK != init_result)
    {
        error_handler("periph_init_all fail");
    }
    // 打印初始化统计信息
    periph_init_print_stats(&init_stats);

    // 打印初始内存报告
    logi("=== System Memory Report at Startup ===");
    memory_print_report();

    ram_func_test();

    logi("=== Application Main Loop Started ===");
    if (worker_thread_init(16, 2048, 4) != 0)
    {
        error_handler("worker_thread_init fail");
    }
    uint32_t cycle_count = 0;

    while (1)
    {
        WORKER_SUBMIT(system_monitor_work, NULL);

        osDelay(1000);
        cycle_count++;
        logi("Main loop cycle #%lu", cycle_count);
        LED_GREEN_TOGGLE();
    }
}

#include "FreeRTOS.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask; // 避免未使用参数警告

    // 栈溢出时不能安全调用任何FreeRTOS API
    // 因为栈已经损坏，调用API可能导致更严重的问题

    // 禁用中断，防止任务切换
    taskDISABLE_INTERRUPTS();

    // 简单的硬件延迟，不依赖FreeRTOS
    volatile uint32_t delay_count;

    while (1)
    {
        LED_RED_TOGGLE();

        // 使用简单的延迟循环，不调用FreeRTOS API
        for (delay_count = 0; delay_count < 1000000; delay_count++)
        {
            __NOP(); // 空操作，防止编译器优化
        }

        // 尝试输出错误信息，但要小心
        // 如果日志系统也依赖栈，这可能失败
        loge("%s stack overflow", pcTaskName ? pcTaskName : "Unknown");

        // 再次延迟
        for (delay_count = 0; delay_count < 1000000; delay_count++)
        {
            __NOP();
        }
    }
}