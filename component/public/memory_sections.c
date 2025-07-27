/**
 * @file memory_sections.c
 * @brief 内存段管理工具函数实现
 * @author MCU Scaffolding Framework
 * @version 1.0
 * @date 2025-07-27
 */

#include "memory_sections.h"
#include "compile.h"
#include <stdio.h>
#include <string.h>

/* 链接器符号声明 - 这些符号在链接器脚本中定义 */
extern uint32_t __ramfunc_start__;
extern uint32_t __ramfunc_end__;
extern uint32_t __ramfunc_load_start__;

extern uint32_t __fastdata_start__;
extern uint32_t __fastdata_end__;
extern uint32_t __fastdata_load_start__;

extern uint32_t __backup_start__;
extern uint32_t __backup_end__;

extern uint32_t __shared_start__;
extern uint32_t __shared_end__;

extern uint32_t __critical_code_start__;
extern uint32_t __critical_code_end__;
extern uint32_t __critical_code_load_start__;

extern uint32_t __critical_data_start__;
extern uint32_t __critical_data_end__;
extern uint32_t __critical_data_load_start__;

extern uint32_t __bootloader_start__;
extern uint32_t __bootloader_end__;

extern uint32_t __config_start__;
extern uint32_t __config_end__;

/* 内存段描述表 */
typedef struct
{
    const char *name;
    uint32_t *start_symbol;
    uint32_t *end_symbol;
    uint32_t *load_start_symbol;
    bool needs_copy;
} section_desc_t;

static const section_desc_t section_table[SECTION_MAX] = {
    [SECTION_RAMFUNC] = {
        .name = "RAMFUNC",
        .start_symbol = &__ramfunc_start__,
        .end_symbol = &__ramfunc_end__,
        .load_start_symbol = &__ramfunc_load_start__,
        .needs_copy = true},
    [SECTION_FASTDATA] = {.name = "FASTDATA", .start_symbol = &__fastdata_start__, .end_symbol = &__fastdata_end__, .load_start_symbol = &__fastdata_load_start__, .needs_copy = true},
    [SECTION_BACKUP] = {.name = "BACKUP", .start_symbol = &__backup_start__, .end_symbol = &__backup_end__, .load_start_symbol = NULL, .needs_copy = false},
    [SECTION_SHARED] = {.name = "SHARED", .start_symbol = &__shared_start__, .end_symbol = &__shared_end__, .load_start_symbol = NULL, .needs_copy = false},
    [SECTION_CRITICAL_CODE] = {.name = "CRITICAL_CODE", .start_symbol = &__critical_code_start__, .end_symbol = &__critical_code_end__, .load_start_symbol = &__critical_code_load_start__, .needs_copy = true},
    [SECTION_CRITICAL_DATA] = {.name = "CRITICAL_DATA", .start_symbol = &__critical_data_start__, .end_symbol = &__critical_data_end__, .load_start_symbol = &__critical_data_load_start__, .needs_copy = true},
    [SECTION_BOOTLOADER] = {.name = "BOOTLOADER", .start_symbol = &__bootloader_start__, .end_symbol = &__bootloader_end__, .load_start_symbol = NULL, .needs_copy = false},
    [SECTION_CONFIG] = {.name = "CONFIG", .start_symbol = &__config_start__, .end_symbol = &__config_end__, .load_start_symbol = NULL, .needs_copy = false}};

/* 段初始化状态 */
static bool sections_initialized = false;

/**
 * @brief 从FLASH复制数据到RAM段
 * @param ram_start RAM起始地址
 * @param flash_start FLASH起始地址
 * @param size 要复制的字节数
 */
static void copy_section_data(uint32_t ram_start, uint32_t flash_start, uint32_t size)
{
    uint8_t *ram_ptr = (uint8_t *)ram_start;
    const uint8_t *flash_ptr = (const uint8_t *)flash_start;

    for (uint32_t i = 0; i < size; i++)
    {
        ram_ptr[i] = flash_ptr[i];
    }
}

bool memory_section_get_info(memory_section_type_t section_type, memory_section_info_t *info)
{
    if (section_type >= SECTION_MAX || info == NULL)
    {
        return false;
    }

    const section_desc_t *section = &section_table[section_type];

    info->name = section->name;
    info->start_addr = (uint32_t)section->start_symbol;
    info->end_addr = (uint32_t)section->end_symbol;
    info->size = info->end_addr - info->start_addr;
    info->load_addr = section->load_start_symbol ? (uint32_t)section->load_start_symbol : 0;
    info->is_loaded = sections_initialized && section->needs_copy;

    return true;
}

void memory_sections_init(void)
{
    if (sections_initialized)
    {
        return;
    }

    printf("Initializing memory sections...\n");

    for (memory_section_type_t i = 0; i < SECTION_MAX; i++)
    {
        const section_desc_t *section = &section_table[i];

        if (section->needs_copy && section->load_start_symbol)
        {
            uint32_t start_addr = (uint32_t)section->start_symbol;
            uint32_t end_addr = (uint32_t)section->end_symbol;
            uint32_t load_addr = (uint32_t)section->load_start_symbol;
            uint32_t size = end_addr - start_addr;

            if (size > 0)
            {
                printf("  Copying %s: 0x%08lX -> 0x%08lX (%lu bytes)\n",
                       section->name, (unsigned long)load_addr, (unsigned long)start_addr, (unsigned long)size);
                copy_section_data(start_addr, load_addr, size);
            }
        }
    }

    sections_initialized = true;
    printf("Memory sections initialization complete.\n");
}

void memory_sections_print_info(void)
{
    printf("\n=== Memory Sections Information ===\n");
    printf("%-15s %-10s %-10s %-8s %-10s %-6s\n",
           "Section", "Start", "End", "Size", "Load", "Copy");
    printf("%-15s %-10s %-10s %-8s %-10s %-6s\n",
           "-------", "-----", "---", "----", "----", "----");

    for (memory_section_type_t i = 0; i < SECTION_MAX; i++)
    {
        memory_section_info_t info;
        if (memory_section_get_info(i, &info))
        {
            printf("%-15s 0x%08lX 0x%08lX %6lu   0x%08lX %s\n",
                   info.name,
                   (unsigned long)info.start_addr,
                   (unsigned long)info.end_addr,
                   (unsigned long)info.size,
                   (unsigned long)info.load_addr,
                   section_table[i].needs_copy ? "Yes" : "No");
        }
    }
    printf("\n");
}

bool memory_section_contains_addr(uint32_t addr, memory_section_type_t section_type)
{
    memory_section_info_t info;
    if (!memory_section_get_info(section_type, &info))
    {
        return false;
    }

    return (addr >= info.start_addr && addr < info.end_addr);
}

float memory_section_get_usage(memory_section_type_t section_type)
{
    memory_section_info_t info;
    if (!memory_section_get_info(section_type, &info) || info.size == 0)
    {
        return 0.0f;
    }

    // 这里简化处理，实际使用率需要根据具体需求实现
    // 可以通过分析符号表或者运行时检测来实现
    return (info.size > 0) ? 100.0f : 0.0f;
}