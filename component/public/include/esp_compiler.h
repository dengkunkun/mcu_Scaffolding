#pragma once

#include <stdint.h>
#include <stdbool.h>

// 编译器相关的宏定义
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

// 避免与STM32 HAL库宏定义冲突
#ifndef SECTION
#define SECTION(name) __attribute__((section(name)))
#endif

#ifndef UNUSED
#define UNUSED __attribute__((unused))
#endif

#define PACKED __attribute__((packed))
#define ALIGNED(n) __attribute__((aligned(n)))
#define NOINLINE __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))

// 常用的数学宏
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(x) ((x) < 0 ? -(x) : (x))

// 位操作宏 - 避免与STM32冲突
#ifndef BIT
#define BIT(n) (1UL << (n))
#endif

#ifndef TOGGLE_BIT
#define TOGGLE_BIT(reg, bit) ((reg) ^= (bit))
#endif

// 数组大小宏
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// 对齐宏
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

// 偏移量宏
#define OFFSET_OF(type, member) ((size_t)&((type *)0)->member)
#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - OFFSET_OF(type, member)))