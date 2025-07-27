/**
 * @file memory_sections_example.c
 * @brief 内存段使用示例
 * @author MCU Scaffolding Framework
 * @version 1.0
 * @date 2025-07-27
 */

#include "compile.h"
#include "memory_sections.h"
#include <stdio.h>
#include <stdint.h>

/* ==================== 使用示例 ==================== */

/* 1. RAM函数示例 - 将频繁调用的函数放到RAM中执行 */
RAMFUNC void critical_timing_function(void)
{
    // 这个函数会被复制到RAM中执行，提高执行速度
    // 适用于中断处理、实时控制等场景
    volatile uint32_t temp = 0;
    for (int i = 0; i < 1000; i++)
    {
        temp += i;
    }
}

/* 2. 快速访问数据示例 */
FASTDATA_ALIGNED(4)
uint32_t fast_buffer[256];                   // 1KB快速访问缓冲区
FASTDATA volatile uint32_t status_flags = 0; // 状态标志

/* 3. 关键代码段示例 - 安全相关或加密函数 */
CRITICAL_CODE uint32_t security_hash_calculate(const uint8_t *data, size_t len)
{
    // 安全相关的代码，放在特殊段中
    uint32_t hash = 0x5A5A5A5A;
    for (size_t i = 0; i < len; i++)
    {
        hash ^= data[i];
        hash = (hash << 1) | (hash >> 31); // 简单的位旋转
    }
    return hash;
}

/* 4. 关键数据段示例 */
CRITICAL_DATA_ALIGNED(4)
const uint32_t encryption_key[4] = {
    0x12345678, 0x9ABCDEF0, 0xFEDCBA98, 0x76543210};

/* 将可变的安全缓冲区放到快速数据段，避免段冲突 */
FASTDATA uint8_t secure_buffer[128];

/* 5. 配置数据示例 - 存储在FLASH中的只读配置 */
typedef struct
{
    uint32_t version;
    uint32_t feature_flags;
    uint32_t timeout_ms;
    uint8_t device_id[16];
} device_config_t;

CONFIG_DATA const device_config_t default_config = {
    .version = 0x00010000,       // v1.0.0
    .feature_flags = 0x0000000F, // 启用前4个功能
    .timeout_ms = 5000,          // 5秒超时
    .device_id = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};

/* 6. 备份数据示例 - 掉电保持数据 */
BACKUP_DATA uint32_t backup_counter = 0;
BACKUP_DATA uint8_t system_state = 0;

/* 7. 共享数据示例 - 多任务共享 */
SHARED_DATA_ALIGNED(4)
uint8_t shared_message_buffer[512];
SHARED_DATA volatile uint32_t shared_write_index = 0;
SHARED_DATA volatile uint32_t shared_read_index = 0;

/* 8. 引导加载器通信数据 */
typedef struct
{
    uint32_t magic;    // 魔术字
    uint32_t command;  // 命令
    uint32_t address;  // 地址
    uint32_t length;   // 长度
    uint32_t checksum; // 校验和
} bootloader_msg_t;

BOOTLOADER_DATA volatile bootloader_msg_t bootloader_msg = {
    .magic = 0xDEADBEEF};

/* ==================== 使用函数示例 ==================== */

/**
 * @brief 初始化示例 - 在系统启动时调用
 */
void memory_sections_example_init(void)
{
    printf("=== Memory Sections Example ===\n");

    // 初始化内存段（复制需要的数据从FLASH到RAM）
    memory_sections_init();

    // 打印内存段信息
    memory_sections_print_info();

    // 初始化快速访问数据
    for (int i = 0; i < 256; i++)
    {
        fast_buffer[i] = i * i;
    }
    status_flags = 0x12345678;

    printf("Fast buffer initialized, first value: %lu\n", fast_buffer[0]);
    printf("Status flags: 0x%08lX\n", status_flags);
}

/**
 * @brief 性能测试示例
 */
void memory_sections_performance_test(void)
{
    printf("\n=== Performance Test ===\n");

    // 测试RAM函数性能
    printf("Testing RAMFUNC performance...\n");

    // 这里可以添加性能测试代码
    critical_timing_function();

    // 测试安全函数
    const char test_data[] = "Hello, Memory Sections!";
    uint32_t hash = security_hash_calculate((const uint8_t *)test_data, sizeof(test_data) - 1);
    printf("Security hash: 0x%08lX\n", hash);
}

/**
 * @brief 配置访问示例
 */
void memory_sections_config_demo(void)
{
    printf("\n=== Configuration Demo ===\n");

    printf("Device config version: 0x%08lX\n", default_config.version);
    printf("Feature flags: 0x%08lX\n", default_config.feature_flags);
    printf("Timeout: %lu ms\n", default_config.timeout_ms);

    printf("Device ID: ");
    for (int i = 0; i < 16; i++)
    {
        printf("%02X ", default_config.device_id[i]);
    }
    printf("\n");
}

/**
 * @brief 地址检查示例
 */
void memory_sections_address_check(void)
{
    printf("\n=== Address Check Demo ===\n");

    // 检查各种数据的地址归属
    printf("fast_buffer address: 0x%08lX - ", (unsigned long)&fast_buffer);
    if (memory_section_contains_addr((uint32_t)&fast_buffer, SECTION_FASTDATA))
    {
        printf("in FASTDATA section\n");
    }
    else
    {
        printf("NOT in FASTDATA section\n");
    }

    printf("default_config address: 0x%08lX - ", (unsigned long)&default_config);
    if (memory_section_contains_addr((uint32_t)&default_config, SECTION_CONFIG))
    {
        printf("in CONFIG section\n");
    }
    else
    {
        printf("NOT in CONFIG section\n");
    }

    printf("backup_counter address: 0x%08lX - ", (unsigned long)&backup_counter);
    if (memory_section_contains_addr((uint32_t)&backup_counter, SECTION_BACKUP))
    {
        printf("in BACKUP section\n");
    }
    else
    {
        printf("NOT in BACKUP section\n");
    }
}

/**
 * @brief 综合示例函数
 */
void memory_sections_comprehensive_demo(void)
{
    memory_sections_example_init();
    memory_sections_performance_test();
    memory_sections_config_demo();
    memory_sections_address_check();

    printf("\n=== Memory Sections Demo Complete ===\n");
}