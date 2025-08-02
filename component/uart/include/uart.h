#pragma once
#include "stddef.h"
#include "stdint.h"

#include "esp_err.h"

// 前向声明FreeRTOS类型
#ifndef INC_FREERTOS_H
typedef uint32_t TickType_t;
#endif

/*
同步发送：等待发送结束
异步发送：写入streambuf（使用freertos的streambuf），由worker线程进行实际发送

同步接收：等待指定数量的数据接收完成后返回
异步接收：接收到指定数量的数据后执行对于的回调函数--hal库的回调函数在中断中执行，这里的中断函数在worker线程中执行，减少中断延迟

循环发送：
    如果存者huart->hdmatx,则使用dma发送
    否则使用中断发送
worker线程：
    阻塞等待一个队列

*/

typedef enum
{
    UART_NUM_0,
    UART_NUM_1,
    UART_NUM_2,
    // 根据设备型号宏添加额外的串口设备
    UART_NUM_MAX,
} uart_port_t;

esp_err_t uart_async_init(uart_port_t port, uint32_t size);

/**
 * @brief Send data to the UART port from a given buffer and length,
 *
 * If the UART driver's parameter 'tx_buffer_size' is set to zero:
 * This function will not return until all the data have been sent out, or at least pushed into TX FIFO.
 *
 * Otherwise, if the 'tx_buffer_size' > 0, this function will return after copying all the data to tx ring buffer,
 * UART ISR will then move data from the ring buffer to TX FIFO gradually.
 *
 * @param uart_num UART port number, the max port number is (UART_NUM_MAX -1).
 * @param src   data buffer address
 * @param size  data length to send
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
int uart_write_bytes(uart_port_t uart_num, const void *src, size_t size);

/**
 * @brief UART read bytes from UART buffer
 *
 * @param uart_num UART port number, the max port number is (UART_NUM_MAX -1).
 * @param buf     pointer to the buffer.
 * @param length  data length
 * @param ticks_to_wait sTimeout, count in RTOS ticks
 *
 * @return
 *     - (-1) Error
 *     - OTHERS (>=0) The number of bytes read from UART buffer
 */
int uart_read_bytes(uart_port_t uart_num, void *buf, uint32_t length, TickType_t ticks_to_wait);

// 等待streambuf写入完毕
esp_err_t uart_flush(uart_port_t uart_num);
// 清空streambuf
esp_err_t uart_clear(uart_port_t uart_num);

/**
 * @brief   UART get RX ring buffer cached data length
 *
 * @param   uart_num UART port number, the max port number is (UART_NUM_MAX -1).
 * @param   size Pointer of size_t to accept cached data length
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t uart_get_buffered_data_len(uart_port_t uart_num, size_t *size);

/**
 * @brief   UART get TX ring buffer free space size
 *
 * @param   uart_num UART port number, the max port number is (UART_NUM_MAX -1).
 * @param   size Pointer of size_t to accept the free space size
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
esp_err_t uart_get_tx_buffer_free_size(uart_port_t uart_num, size_t *size);

void uart3_test();