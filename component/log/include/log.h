#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "elog.h"

#ifdef ELOG_OUTPUT_DIR
#undef ELOG_OUTPUT_DIR
#include <string.h>
#define SHORT_FILE_NAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#define ELOG_OUTPUT_DIR SHORT_FILE_NAME
#endif

/* 日志级别定义 */
#define LOG_ASSERT  ELOG_LVL_ASSERT
#define LOG_ERROR   ELOG_LVL_ERROR  
#define LOG_WARN    ELOG_LVL_WARN   
#define LOG_INFO    ELOG_LVL_INFO   
#define LOG_DEBUG   ELOG_LVL_DEBUG  
#define LOG_VERBOSE ELOG_LVL_VERBOSE

/* 便捷的日志宏定义 */
#define loga(...)   elog_a(__func__, __VA_ARGS__)
#define loge(...)   elog_e(__func__, __VA_ARGS__)
#define logw(...)   elog_w(__func__, __VA_ARGS__)
#define logi(...)   elog_i(__func__, __VA_ARGS__)
#define logd(...)   elog_d(__func__, __VA_ARGS__)
#define logv(...)   elog_v(__func__, __VA_ARGS__)

/* 原始输出宏（不带级别格式） */
#define log_raw(...)      elog_raw(__VA_ARGS__)

/* 十六进制数据输出 */
#define log_hex(tag, width, buf, size)  elog_hexdump(tag, width, buf, size)

/* 初始化和控制函数 */
/**
 * @brief 初始化日志系统
 * @return int 0-成功，其他-失败
 */
int log_init(void);

/**
 * @brief 启动日志输出
 */
void log_start(void);

/**
 * @brief 停止日志输出  
 */
void log_stop(void);

/**
 * @brief 设置日志输出级别
 * @param level 日志级别
 */
void log_set_output_level(uint8_t level);

/**
 * @brief 获取当前日志输出级别
 * @return uint8_t 当前级别
 */
uint8_t log_get_output_level(void);

/**
 * @brief 设置日志过滤标签
 * @param tag 标签名，NULL表示清除过滤
 */
void log_set_filter_tag(const char *tag);

/**
 * @brief 设置日志过滤关键字
 * @param keyword 关键字，NULL表示清除过滤
 */
void log_set_filter_kw(const char *keyword);

/**
 * @brief 刷新日志缓冲区
 */
void log_flush(void);

/* 性能统计宏 */
#if defined(ELOG_TIME_ENABLE) && ELOG_TIME_ENABLE
#define LOG_PERF_START(tag) \
    do { \
        static uint32_t __perf_start_time; \
        __perf_start_time = elog_get_time(); \
        log_d(tag, "Performance start");

#define LOG_PERF_END(tag) \
        uint32_t __perf_end_time = elog_get_time(); \
        log_i(tag, "Performance: %lu ms", __perf_end_time - __perf_start_time); \
    } while(0)
#else
#define LOG_PERF_START(tag)
#define LOG_PERF_END(tag)
#endif

/* 条件日志宏 */
#define log_if(condition, level, tag, ...) \
    do { \
        if (condition) { \
            log_##level(tag, __VA_ARGS__); \
        } \
    } while(0)

/* 频率限制日志宏（每N次调用输出一次） */
#define log_every_n(n, level, tag, ...) \
    do { \
        static uint32_t __log_count = 0; \
        if (++__log_count % (n) == 1) { \
            log_##level(tag, __VA_ARGS__); \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif /* __LOG_H__ */