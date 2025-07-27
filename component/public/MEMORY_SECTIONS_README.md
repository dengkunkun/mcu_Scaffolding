# 内存段管理功能使用指南

## 概述

本功能提供了一套完整的内存段管理系统，允许开发者将函数和变量放置到特定的内存区域中，以实现性能优化、安全隔离、配置管理等需求。

## 功能特性

- **RAM函数段 (RAMFUNC)**: 将函数复制到RAM中执行，提高性能
- **快速数据段 (FASTDATA)**: 用于需要高速访问的数据
- **关键代码段 (CRITICAL_CODE)**: 用于安全敏感或时间关键的代码
- **关键数据段 (CRITICAL_DATA)**: 用于重要的配置或状态数据
- **配置数据段 (CONFIG)**: 存储在FLASH中的只读配置信息
- **备份数据段 (BACKUP)**: 用于掉电保持或特殊用途的数据
- **共享数据段 (SHARED)**: 用于多任务间的共享内存
- **引导加载器段 (BOOTLOADER)**: 用于引导加载器通信

## 使用方法

### 1. 包含头文件

```c
#include "compile.h"          // 基本宏定义
#include "memory_sections.h"  // 内存段管理功能
```

### 2. 使用预定义宏

```c
// 将函数放到RAM中执行
RAMFUNC void fast_function(void) {
    // 高性能代码
}

// 将数据放到快速访问段
FASTDATA uint32_t critical_buffer[256];

// 关键代码段
CRITICAL_CODE void security_function(void) {
    // 安全相关代码
}

// 配置数据（存储在FLASH中）
CONFIG_DATA const config_t device_config = {
    .version = 0x00010000,
    .timeout = 5000
};

// 备份数据（RAM中但特殊用途）
BACKUP_DATA uint32_t backup_counter = 0;
```

### 3. 组合使用对齐宏

```c
// RAM函数 + 4字节对齐
RAMFUNC_ALIGNED(4) void aligned_fast_function(void) {
    // 对齐的高性能函数
}

// 快速数据 + 32字节对齐（适用于DMA）
FASTDATA_ALIGNED(32) uint8_t dma_buffer[1024];
```

### 4. 自定义内存段

```c
// 使用通用SECTION宏定义自定义段
SECTION(".my_custom_section") int my_variable = 42;
```

## 系统集成

### 1. 初始化内存段

在系统启动早期调用初始化函数：

```c
int main(void) {
    // HAL初始化...
    
    // 初始化内存段（复制FLASH中的数据到RAM段）
    memory_sections_init();
    
    // 其他初始化...
}
```

### 2. 获取段信息

```c
void check_memory_layout(void) {
    memory_section_info_t info;
    
    // 获取RAMFUNC段信息
    if (memory_section_get_info(SECTION_RAMFUNC, &info)) {
        printf("RAMFUNC: 0x%08X - 0x%08X (%lu bytes)\n",
               info.start_addr, info.end_addr, info.size);
    }
    
    // 打印所有段信息
    memory_sections_print_info();
}
```

### 3. 地址检查

```c
void address_validation(void) {
    uint32_t addr = (uint32_t)&my_fast_data;
    
    if (memory_section_contains_addr(addr, SECTION_FASTDATA)) {
        printf("Address is in FASTDATA section\n");
    }
}
```

## 链接器脚本集成

系统已自动修改链接器脚本 `STM32F103XX_FLASH.ld`，添加了以下内存段：

- `.ramfunc`: RAM函数段 (VMA在RAM，LMA在FLASH)
- `.fastdata`: 快速数据段 (VMA在RAM，LMA在FLASH)  
- `.critical_code`: 关键代码段 (VMA在RAM，LMA在FLASH)
- `.critical_data`: 关键数据段 (VMA在RAM，LMA在FLASH)
- `.config`: 配置数据段 (存储在FLASH)
- `.backup`: 备份数据段 (仅在RAM)
- `.shared`: 共享数据段 (仅在RAM)
- `.bootloader`: 引导加载器段 (仅在RAM)

## 性能考虑

### RAM函数的优势
- 无FLASH访问等待时间
- 适用于中断处理程序
- 适用于实时控制算法

### 使用建议
- 仅将关键函数放入RAM段，避免浪费RAM空间
- 大型数据结构优先考虑FLASH存储
- 使用对齐宏优化DMA和缓存性能

## 调试技巧

### 1. 查看内存映射
编译后查看 `.map` 文件中的段分布：
```
.ramfunc        0x20000100   0x200001ff   255
.fastdata       0x20000200   0x200003ff   511
```

### 2. 运行时检查
```c
void debug_memory_sections(void) {
    memory_sections_print_info();
    
    // 检查函数地址
    printf("Function address: 0x%08X\n", (uint32_t)&my_ram_function);
}
```

## 示例代码

参考 `memory_sections_example.c` 文件获取完整的使用示例，包括：
- 各种内存段的使用方法
- 性能测试代码
- 地址验证示例
- 配置数据访问示例

## 注意事项

1. **初始化顺序**: 必须在使用RAM段数据前调用 `memory_sections_init()`
2. **内存限制**: 注意RAM使用量，避免超出系统限制
3. **链接器兼容**: 确保链接器脚本与代码版本匹配
4. **调试支持**: 某些调试器可能需要额外配置才能正确显示段信息

## 扩展开发

如需添加新的内存段类型：

1. 在 `compile.h` 中添加新的宏定义
2. 在 `memory_sections.h` 中添加新的枚举值
3. 在 `memory_sections.c` 中添加对应的段描述
4. 在链接器脚本中添加新的段定义
5. 更新示例代码和文档