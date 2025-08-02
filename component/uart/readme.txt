# UART 异步通信组件

本组件实现了类似ESP32的UART接口，为STM32提供异步串口通信功能。

## 特性

- **ESP32风格API** - 熟悉的接口设计，降低学习成本
- **异步架构** - 基于FreeRTOS Stream Buffer和Worker线程
- **硬件抽象** - 支持多个UART端口
- **DMA支持** - 自动检测并使用DMA传输
- **中断延迟优化** - 中断处理最小化，主要逻辑在Worker线程中执行
- **缓冲区管理** - 灵活的发送和接收缓冲区大小配置

## 架构设计

```
用户应用层
    ↓
UART API层 (uart.h)
    ↓
Stream Buffer层 (FreeRTOS)
    ↓
Worker Thread层
    ↓
HAL驱动层 (STM32 HAL)
    ↓
硬件层 (UART外设)
```

## API参考

### 初始化

```c
esp_err_t uart_async_init(uart_port_t port, uint32_t size);
```

初始化指定的UART端口。

**参数:**
- `port`: UART端口号 (UART_NUM_0, UART_NUM_1, UART_NUM_2)
- `size`: 缓冲区大小（字节）

**返回值:**
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 无效参数
- `ESP_ERR_INVALID_STATE`: 端口已初始化
- `ESP_ERR_NO_MEM`: 内存不足

### 数据发送

```c
int uart_write_bytes(uart_port_t uart_num, const void *src, size_t size);
```

发送数据到UART端口。该函数是异步的，数据会写入发送缓冲区后立即返回。

**参数:**
- `uart_num`: UART端口号
- `src`: 要发送的数据
- `size`: 数据长度

**返回值:**
- `>= 0`: 实际写入缓冲区的字节数
- `-1`: 错误

### 数据接收

```c
int uart_read_bytes(uart_port_t uart_num, void *buf, uint32_t length, TickType_t ticks_to_wait);
```

从UART端口接收数据。

**参数:**
- `uart_num`: UART端口号
- `buf`: 接收缓冲区
- `length`: 期望接收的字节数
- `ticks_to_wait`: 超时时间（FreeRTOS ticks）

**返回值:**
- `> 0`: 实际接收的字节数
- `0`: 超时
- `-1`: 错误

### 缓冲区控制

```c
esp_err_t uart_flush(uart_port_t uart_num);        // 等待发送完成
esp_err_t uart_clear(uart_port_t uart_num);        // 清空缓冲区
esp_err_t uart_get_buffered_data_len(uart_port_t uart_num, size_t *size);  // 获取接收缓冲区数据长度
esp_err_t uart_get_tx_buffer_free_size(uart_port_t uart_num, size_t *size); // 获取发送缓冲区空闲空间
```

## 使用示例

### 基本使用

```c
#include "uart.h"

void app_main(void) {
    // 初始化UART0，缓冲区大小1024字节
    esp_err_t ret = uart_async_init(UART_NUM_0, 1024);
    if (ret != ESP_OK) {
        printf("UART初始化失败\n");
        return;
    }
    
    // 发送数据
    const char *data = "Hello, UART!\n";
    uart_write_bytes(UART_NUM_0, data, strlen(data));
    
    // 接收数据
    uint8_t rx_buffer[128];
    int len = uart_read_bytes(UART_NUM_0, rx_buffer, sizeof(rx_buffer) - 1, pdMS_TO_TICKS(1000));
    if (len > 0) {
        rx_buffer[len] = '\0';
        printf("收到: %s\n", rx_buffer);
    }
}
```

### 高级使用：命令行处理器

```c
void uart_command_processor(void) {
    char cmd_buffer[128];
    int len;
    
    uart_write_bytes(UART_NUM_0, "CMD> ", 5);
    
    while (1) {
        len = uart_read_bytes(UART_NUM_0, cmd_buffer, sizeof(cmd_buffer) - 1, portMAX_DELAY);
        if (len > 0) {
            cmd_buffer[len] = '\0';
            
            // 处理命令
            if (strncmp(cmd_buffer, "status", 6) == 0) {
                size_t rx_len, tx_free;
                uart_get_buffered_data_len(UART_NUM_0, &rx_len);
                uart_get_tx_buffer_free_size(UART_NUM_0, &tx_free);
                
                char status[64];
                snprintf(status, sizeof(status), "RX: %zu, TX_free: %zu\n", rx_len, tx_free);
                uart_write_bytes(UART_NUM_0, status, strlen(status));
            }
            
            uart_write_bytes(UART_NUM_0, "CMD> ", 5);
        }
    }
}
```

## 配置要求

### HAL配置

确保在HAL配置中启用了需要的UART外设：

```c
// 在main.c或相应的初始化文件中定义UART句柄
UART_HandleTypeDef huart1;  // 对应UART_NUM_0
UART_HandleTypeDef huart2;  // 对应UART_NUM_1  
UART_HandleTypeDef huart3;  // 对应UART_NUM_2
```

### FreeRTOS配置

确保启用了Stream Buffer功能：

```c
#define configUSE_STREAM_BUFFERS    1
#define configSUPPORT_DYNAMIC_ALLOCATION    1
```

### Worker组件

本组件依赖Worker组件来处理发送任务，确保Worker组件已正确初始化。

## 移植说明

如果需要支持更多UART端口，修改以下内容：

1. 在`uart.h`中添加新的端口枚举：
```c
typedef enum {
    UART_NUM_0,
    UART_NUM_1,
    UART_NUM_2,
    UART_NUM_3,    // 新增
    UART_NUM_MAX,
} uart_port_t;
```

2. 在`uart.c`中的`get_hal_uart_handle()`函数添加新的映射：
```c
case UART_NUM_3:
    return &huart4;  // 对应的HAL句柄
```

3. 在项目中定义对应的HAL句柄：
```c
extern UART_HandleTypeDef huart4;
```

## 性能特点

- **低中断延迟**: 中断处理仅做必要的数据传输，复杂逻辑在任务中处理
- **高并发**: 支持多个UART端口同时工作
- **内存效率**: 使用Stream Buffer提供高效的数据缓冲
- **DMA优化**: 自动检测并使用DMA传输，减少CPU占用

## 注意事项

1. 确保Worker组件在UART组件之前初始化
2. 发送大量数据时注意缓冲区大小，避免阻塞
3. 接收数据时适当设置超时时间
4. 在中断回调函数中不要执行耗时操作
5. 多任务环境下注意线程安全（API内部已处理）

## 故障排除

### 常见问题

**Q: 初始化失败，返回ESP_ERR_NOT_FOUND**
A: 检查HAL UART句柄是否正确定义和初始化

**Q: 数据发送不出去**
A: 检查Worker组件是否正常工作，GPIO是否正确配置

**Q: 接收不到数据**
A: 检查接收中断是否正常触发，波特率是否匹配

**Q: 系统卡死**
A: 检查是否在中断中调用了阻塞函数，确保正确使用FromISR版本的API