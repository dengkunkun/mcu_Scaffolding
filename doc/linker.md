# 链接器脚本常用语法介绍

基于您的STM32F103链接器脚本，我来介绍链接器脚本的常用语法：

## 1. 基本结构

````ld
/* 注释 */
ENTRY(Reset_Handler)        /* 入口点 */
MEMORY { ... }              /* 内存区域定义 */
SECTIONS { ... }            /* 段定义 */
````

## 2. 符号定义与赋值

````ld
/* 简单赋值 */
_estack = ORIGIN(RAM) + LENGTH(RAM);    /* 栈顶地址 */
_Min_Heap_Size = 0x2000;                /* 堆大小常量 */

/* 位置计数器赋值 */
. = ALIGN(4);                           /* 对齐到4字节 */
. = . + _Min_Heap_Size;                 /* 增加偏移 */
````

## 3. PROVIDE 语法

`PROVIDE` 用于提供符号定义，它不占用任何存储空间，仅在该符号未被其他地方定义时生效，如果在其他位置定义，则被覆盖：
在链接时提供一个地址标记，不占用内存
````ld
/* PROVIDE 语法示例 */
PROVIDE ( end = . );            /* 提供 end 符号  */
PROVIDE ( _end = . );           /* 提供 _end 符号  */

/* PROVIDE_HIDDEN - 提供隐藏符号（不导出到符号表）*/
PROVIDE_HIDDEN (__preinit_array_start = .);
PROVIDE_HIDDEN (__preinit_array_end = .);
````

**PROVIDE vs 直接赋值的区别：**
- `symbol = value;` - 强制定义，会覆盖已存在的定义
- `PROVIDE(symbol = value);` - 条件定义，仅在符号未定义时生效

## 4. MEMORY 区域定义

````ld
MEMORY
{
  /* 名称 (属性) : 起始地址 = 起始值, 长度 = 大小 */
  RAM (xrw)   : ORIGIN = 0x20000000, LENGTH = 64K
  FLASH (rx)  : ORIGIN = 0x8000000,  LENGTH = 512K
}

/* 属性说明：
 * r - 可读
 * w - 可写  
 * x - 可执行
 * a - 可分配
 * i - 初始化
 * l - 与输入段相同属性
 */
````

## 5. SECTIONS 段定义

````ld
SECTIONS
{
  /* 输出段名 : */
  .text :
  {
    . = ALIGN(4);              /* 对齐 */
    *(.text)                   /* 收集所有 .text 段 */
    *(.text*)                  /* 收集所有 .text.* 段 */
    . = ALIGN(4);
    _etext = .;               /* 定义段结束符号 */
  } >FLASH                    /* 放置到 FLASH 区域 */
  
  /* 数据段 - VMA 和 LMA 不同 */
  .data :
  {
    _sdata = .;
    *(.data)
    *(.data*)
    _edata = .;
  } >RAM AT> FLASH            /* VMA在RAM，LMA在FLASH */
}
````

## 6. 常用内置函数

````ld
/* 内存区域函数 */
ORIGIN(memory)                 /* 获取内存区域起始地址 */
LENGTH(memory)                 /* 获取内存区域长度 */

/* 段相关函数 */
LOADADDR(section)              /* 获取段的加载地址(LMA) */
ADDR(section)                  /* 获取段的虚拟地址(VMA) */
SIZEOF(section)                /* 获取段的大小 */

/* 对齐函数 */
ALIGN(exp)                     /* 对齐到指定边界 */
ALIGN(exp1, exp2)              /* 将exp1对齐到exp2边界 */
````

## 7. 通配符和模式匹配

````ld
/* 通配符 */
*(.text)          /* 所有文件的 .text 段 */
*(.text*)         /* 所有文件的 .text.* 段 */
file.o(.text)     /* 特定文件的 .text 段 */

/* 排序 */
SORT(.init_array.*)           /* 按名称排序 */
SORT_BY_ALIGNMENT(.data*)     /* 按对齐方式排序 */
````

## 8. KEEP 关键字

防止链接器优化时删除未引用的段：

````ld
KEEP(*(.isr_vector))          /* 保持中断向量表 */
KEEP (*(.init))               /* 保持初始化代码 */
KEEP (*(SORT(.init_array.*))) /* 保持并排序初始化数组 */
````

## 9. 条件语句

````ld
/* 简单条件 */
symbol = condition ? value1 : value2;

/* ASSERT 断言 */
ASSERT(condition, "error message");

/* 示例：检查内存溢出 */
ASSERT((_end <= (ORIGIN(RAM) + LENGTH(RAM))), "RAM overflow!")
````

## 10. 特殊符号

````ld
.                    /* 位置计数器（当前地址）*/
_estack             /* 自定义符号（栈顶）*/
__bss_start__       /* 通常用双下划线的系统符号 */

/* 常见的预定义符号 */
_sdata, _edata      /* 数据段开始/结束 */
_sbss, _ebss        /* BSS段开始/结束 */
_sidata             /* 数据段加载地址 */
````

## 11. 完整示例分析

基于您的链接器脚本，这里是一个带注释的关键部分：

````ld
/* 堆栈检查段 */
._user_heap_stack :
{
  . = ALIGN(8);                    /* 8字节对齐 */
  PROVIDE ( end = . );             /* 提供堆起始地址符号 */
  PROVIDE ( _end = . );            /* 兼容性符号 */
  . = . + _Min_Heap_Size;          /* 分配堆空间 */
  . = . + _Min_Stack_Size;         /* 分配栈空间 */
  . = ALIGN(8);                    /* 结束对齐 */
} >RAM                             /* 放置在RAM中 */

/* 如果超出RAM范围，链接器会报错 */
````

## 12. 调试技巧

````ld
/* 添加调试符号 */
.debug_info     0 : { *(.debug_info) }
.debug_line     0 : { *(.debug_line) }

/* 生成符号映射 */
/* 使用 -Wl,-Map=output.map 生成映射文件 */

/* 链接时检查 */
ASSERT((SIZEOF(.data) + SIZEOF(.bss)) <= LENGTH(RAM), "RAM overflow")
````

这些语法元素组合使用，可以精确控制程序在内存中的布局，确保代码、数据和栈堆的合理分配。