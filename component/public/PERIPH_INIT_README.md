# 外设自动初始化框架 (Peripheral Auto-Initialization Framework)

## 概述

这是一个用于STM32嵌入式系统的外设自动初始化框架，允许各个模块自动注册其初始化函数，并在系统启动时按优先级顺序自动执行初始化，无需在main函数中手动调用每个外设的初始化函数。

## 核心特性

- ✅ **自动注册机制**：使用宏自动将初始化函数注册到特殊链接段
- ✅ **优先级控制**：支持按优先级顺序初始化外设
- ✅ **错误处理**：统计初始化成功/失败数量，提供详细错误信息
- ✅ **灵活管理**：支持按名称、优先级范围等方式管理外设初始化
- ✅ **零运行时开销**：所有描述符在编译时确定，运行时只是简单的函数调用
- ✅ **易于调试**：提供详细的统计信息和日志输出

## 文件结构

```
component/public/
├── periph_init.h              # 框架头文件
├── periph_init.c              # 框架实现
├── periph_init_example.c      # 完整使用示例
└── PERIPH_INIT_README.md      # 本文档
```

## 工作原理

### 1. 链接器段机制

框架使用自定义的`.periph_init`段来收集所有外设初始化描述符：

```c
// 在链接器脚本中定义
.periph_init (READONLY) :
{
    . = ALIGN(4);
    PROVIDE_HIDDEN (__periph_init_start = .);
    KEEP (*(SORT(.periph_init.*)))  // 按优先级排序
    KEEP (*(.periph_init*))
    PROVIDE_HIDDEN (__periph_init_end = .);
    . = ALIGN(4);
} >FLASH
```

### 2. 宏注册机制

使用`REGISTER_PERIPH_INIT`宏将初始化描述符放入特殊段：

```c
#define REGISTER_PERIPH_INIT(name_str, init_function, init_param, init_priority) \
    static const periph_init_desc_t __periph_init_desc_##init_function \
    __attribute__((section(".periph_init." #init_priority), used)) = { \
        .name = name_str, \
        .init_func = init_function, \
        .param = init_param, \
        .priority = init_priority, \
        .reserved = 0 \
    }
```

### 3. 自动执行机制

在main函数中调用`periph_init_all()`扫描所有描述符并按优先级执行：

```c
periph_init_result_t periph_init_all(periph_init_stats_t *stats);
```

## 快速开始

### 1. 链接器脚本配置

在你的链接器脚本中添加`.periph_init`段（已在`STM32F103XX_FLASH.ld`中完成）。

### 2. 基本使用示例

```c
#include "periph_init.h"

// 1. 定义初始化函数
static int uart1_init(void *param) {
    UART_HandleTypeDef *huart = (UART_HandleTypeDef*)param;
    
    huart->Instance = USART1;
    huart->Init.BaudRate = 115200;
    // ... 其他配置
    
    if (HAL_UART_Init(huart) != HAL_OK) {
        return -1; // 失败
    }
    
    return 0; // 成功
}

// 2. 定义配置参数（可选）
static UART_HandleTypeDef huart1;

// 3. 注册初始化函数
REGISTER_PERIPH_INIT("UART1", uart1_init, &huart1, 200);

// 4. 在main函数中执行所有初始化
int main(void) {
    // ... 系统基础初始化（时钟等）
    
    periph_init_stats_t stats;
    periph_init_result_t result = periph_init_all(&stats);
    
    if (result == PERIPH_INIT_OK) {
        printf("所有外设初始化成功!\n");
    } else {
        printf("外设初始化出现问题\n");
    }
    
    // ... 启动FreeRTOS或进入主循环
}
```

### 3. 无参数初始化

对于不需要参数的初始化函数：

```c
static int led_init(void *param) {
    (void)param; // 未使用参数
    
    // LED初始化代码
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    
    return 0;
}

// 使用简化宏
REGISTER_PERIPH_INIT_SIMPLE("LED", led_init, 100);
```

## API参考

### 核心数据结构

```c
// 初始化函数类型
typedef int (*periph_init_func_t)(void *param);

// 初始化描述符
typedef struct {
    const char *name;              // 外设名称
    periph_init_func_t init_func;  // 初始化函数
    void *param;                   // 参数指针
    uint16_t priority;             // 优先级
    uint16_t reserved;             // 保留字段
} periph_init_desc_t;

// 初始化结果
typedef enum {
    PERIPH_INIT_OK = 0,           // 全部成功
    PERIPH_INIT_PARTIAL_FAIL,     // 部分失败
    PERIPH_INIT_ALL_FAIL,         // 全部失败
    PERIPH_INIT_NO_DEVICES        // 无设备
} periph_init_result_t;

// 统计信息
typedef struct {
    uint32_t total_count;         // 总数量
    uint32_t success_count;       // 成功数量
    uint32_t failed_count;        // 失败数量
    const char *first_failed_name; // 第一个失败的设备名
} periph_init_stats_t;
```

### 主要API函数

```c
// 执行所有外设初始化
periph_init_result_t periph_init_all(periph_init_stats_t *stats);

// 打印统计信息
void periph_init_print_stats(const periph_init_stats_t *stats);

// 按名称初始化特定外设
int periph_init_by_name(const char *name);

// 按优先级范围初始化
periph_init_result_t periph_init_by_priority_range(uint16_t min_priority, 
                                                  uint16_t max_priority,
                                                  periph_init_stats_t *stats);

// 获取描述符列表（调试用）
const periph_init_desc_t* periph_init_get_descriptors(uint32_t *count);
```

### 注册宏

```c
// 完整版本（带参数）
REGISTER_PERIPH_INIT(name_str, init_function, init_param, init_priority)

// 简化版本（无参数）
REGISTER_PERIPH_INIT_SIMPLE(name_str, init_function, init_priority)
```

## 优先级规划

建议按以下规则分配优先级：

| 优先级范围 | 用途 | 示例 |
|------------|------|------|
| 0-99 | 系统基础设施 | 时钟、电源管理、看门狗 |
| 100-199 | 基础外设 | GPIO、DMA、基础定时器 |  
| 200-299 | 通信外设 | UART、SPI、I2C、CAN |
| 300-399 | 高级外设 | ADC、DAC、PWM、高级定时器 |
| 400-499 | 应用层外设 | 传感器、显示器、存储器 |
| 500+ | 用户自定义 | 应用相关的初始化 |

## 高级用法

### 1. 分阶段初始化

```c
// 第一阶段：基础外设初始化
periph_init_by_priority_range(0, 199, &stats1);

// 第二阶段：通信外设初始化
periph_init_by_priority_range(200, 299, &stats2);

// 第三阶段：应用外设初始化
periph_init_by_priority_range(300, 999, &stats3);
```

### 2. 错误处理策略

```c
periph_init_result_t result = periph_init_all(&stats);

switch (result) {
    case PERIPH_INIT_OK:
        // 继续正常启动
        start_application();
        break;
        
    case PERIPH_INIT_PARTIAL_FAIL:
        // 记录错误，可能降级运行
        log_error("Some peripherals failed, running in degraded mode");
        start_application_safe_mode();
        break;
        
    case PERIPH_INIT_ALL_FAIL:
        // 严重错误，进入错误模式
        enter_error_mode();
        break;
        
    case PERIPH_INIT_NO_DEVICES:
        // 配置问题，检查链接器脚本
        log_warning("No devices registered");
        break;
}
```

### 3. 运行时重新初始化

```c
// 按名称重新初始化特定外设
int result = periph_init_by_name("UART1");
if (result != 0) {
    printf("UART1 re-initialization failed\n");
}

// 重新初始化所有通信外设
periph_init_by_priority_range(200, 299, NULL);
```

## 调试和监控

### 1. 获取外设列表

```c
uint32_t count;
const periph_init_desc_t *descs = periph_init_get_descriptors(&count);

printf("Registered peripherals (%lu):\n", count);
for (uint32_t i = 0; i < count; i++) {
    printf("  %s (priority: %u)\n", descs[i].name, descs[i].priority);
}
```

### 2. 统计信息分析

```c
periph_init_stats_t stats;
periph_init_all(&stats);

printf("Total: %lu, Success: %lu, Failed: %lu\n",
       stats.total_count, stats.success_count, stats.failed_count);

if (stats.failed_count > 0) {
    printf("First failed device: %s\n", stats.first_failed_name);
}

float success_rate = (float)stats.success_count / stats.total_count * 100.0f;
printf("Success rate: %.1f%%\n", success_rate);
```

## 最佳实践

1. **命名规范**：使用清晰、一致的外设名称
2. **优先级规划**：合理分配优先级，避免循环依赖
3. **错误处理**：初始化函数应返回明确的错误码
4. **参数管理**：使用静态变量存储配置参数
5. **日志输出**：在初始化函数中添加适当的日志
6. **测试覆盖**：测试各种初始化失败场景

## 注意事项

1. **链接器脚本**：必须正确配置`.periph_init`段
2. **内存对齐**：描述符结构体已考虑内存对齐
3. **线程安全**：框架本身不是线程安全的，应在单线程环境中使用
4. **重复初始化**：框架不防止重复初始化，需要应用层处理
5. **错误恢复**：初始化失败时需要应用层决定恢复策略

## 故障排除

### 问题：没有找到任何外设描述符

**可能原因：**
- 链接器脚本中没有正确定义`.periph_init`段
- 编译器优化导致描述符被删除

**解决方案：**
- 检查链接器脚本配置
- 确保使用了`used`属性
- 检查编译器和链接器选项

### 问题：初始化顺序不正确

**可能原因：**
- 优先级设置错误
- 链接器没有按优先级排序

**解决方案：**
- 检查优先级数值（越小越优先）
- 确保链接器脚本使用了`SORT(.periph_init.*)`

### 问题：某些外设初始化失败

**可能原因：**
- 硬件连接问题
- 时钟配置问题
- 依赖的外设未初始化

**解决方案：**
- 检查硬件连接
- 确认时钟配置正确
- 调整初始化优先级解决依赖问题

## 示例项目

完整的使用示例请参考：
- `periph_init_example.c` - 各种外设初始化示例
- `app_main.c` - 在实际项目中的集成示例

## 许可证

本框架遵循项目的整体许可证。