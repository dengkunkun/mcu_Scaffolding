/**
 * @file memory_sections.h
 * @brief 内存段管理工具函数和类型定义
 * @author MCU Scaffolding Framework
 * @version 1.0
 * @date 2025-07-27
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* 内存段信息结构体 */
    typedef struct
    {
        const char *name;    /* 段名称 */
        uint32_t start_addr; /* 起始地址 */
        uint32_t end_addr;   /* 结束地址 */
        uint32_t size;       /* 段大小 */
        uint32_t load_addr;  /* 加载地址（LMA）*/
        bool is_loaded;      /* 是否已加载到RAM */
    } memory_section_info_t;

    /* 内存段类型枚举 */
    typedef enum
    {
        SECTION_RAMFUNC = 0,   /* RAM函数段 */
        SECTION_FASTDATA,      /* 快速数据段 */
        SECTION_BACKUP,        /* 备份数据段 */
        SECTION_SHARED,        /* 共享数据段 */
        SECTION_CRITICAL_CODE, /* 关键代码段 */
        SECTION_CRITICAL_DATA, /* 关键数据段 */
        SECTION_BOOTLOADER,    /* 引导加载器段 */
        SECTION_CONFIG,        /* 配置数据段 */
        SECTION_MAX            /* 段数量 */
    } memory_section_type_t;

    /**
     * @brief 获取指定内存段的信息
     * @param section_type 段类型
     * @param info 输出的段信息
     * @return true 成功，false 失败
     */
    bool memory_section_get_info(memory_section_type_t section_type, memory_section_info_t *info);

    /**
     * @brief 初始化所有需要从FLASH复制到RAM的段
     * @note 这个函数应该在系统启动早期调用
     */
    void memory_sections_init(void);

    /**
     * @brief 打印所有内存段的信息（调试用）
     */
    void memory_sections_print_info(void);

    /**
     * @brief 检查地址是否在指定段内
     * @param addr 要检查的地址
     * @param section_type 段类型
     * @return true 在段内，false 不在段内
     */
    bool memory_section_contains_addr(uint32_t addr, memory_section_type_t section_type);

    /**
     * @brief 获取段的使用率
     * @param section_type 段类型
     * @return 使用率百分比（0-100）
     */
    float memory_section_get_usage(memory_section_type_t section_type);

#ifdef __cplusplus
}
#endif