/*
******************************************************************************
**

**  File        : LinkerScript.ld
**
**  Author		: STM32CubeMX
**
**  Abstract    : Linker script for STM32F103ZETx series
**                512Kbytes FLASH and 64Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed “as is,” without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** <h2><center>&copy; COPYRIGHT(c) 2025 STMicroelectronics</center></h2>
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**   1. Redistributions of source code must retain the above copyright notice,
**      this list of conditions and the following disclaimer.
**   2. Redistributions in binary form must reproduce the above copyright notice,
**      this list of conditions and the following disclaimer in the documentation
**      and/or other materials provided with the distribution.
**   3. Neither the name of STMicroelectronics nor the names of its contributors
**      may be used to endorse or promote products derived from this software
**      without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);    /* end of RAM */

/* Generate a link error if heap and stack don't fit into RAM */
/* 优化后的堆栈配置 - 基于实际使用情况调整 */
_Min_Heap_Size = 0x800;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* 内存使用计算 (STM32F103ZET6 - 64KB RAM):
 * - Global variables (.data + .bss): ~11KB
 * - FreeRTOS heap: 8KB (configTOTAL_HEAP_SIZE)
 * - Main stack: 2KB (启动和中断使用)
 * - 剩余空间: ~43KB (用于FreeRTOS任务栈等)
 */

/* Specify the memory areas */
MEMORY
{
RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 64K
FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 512K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH

  .ARM (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  .preinit_array (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array (READONLY) : /* The "READONLY" keyword is only supported in GCC11 and later, remove it if using GCC10 or earlier. */
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  /* 外设初始化段 - 自动收集外设初始化描述符 */
  .periph_init (READONLY) :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__periph_init_start = .);
    /* 按优先级排序收集外设初始化描述符 */
    KEEP (*(SORT(.periph_init.*)))
    KEEP (*(.periph_init*))
    PROVIDE_HIDDEN (__periph_init_end = .);
    . = ALIGN(4);
  } >FLASH

  /* 配置数据段（只读） - 存储在FLASH中的配置信息 */
  .config (READONLY) :
  {
    . = ALIGN(4);
    __config_start__ = .;
    *(.config)
    *(.config*)
    . = ALIGN(4);
    __config_end__ = .;
  } >FLASH
  __config_size__ = __config_end__ - __config_start__;

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT> FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* ==================== 自定义内存段定义（在数据段之后）==================== */
  
  /* RAM函数段 - 将函数代码复制到RAM中执行以提高性能 */
  .ramfunc :
  {
    . = ALIGN(4);
    __ramfunc_start__ = .;
    *(.ramfunc)
    *(.ramfunc*)
    . = ALIGN(4);
    __ramfunc_end__ = .;
  } >RAM AT> FLASH
  __ramfunc_load_start__ = LOADADDR(.ramfunc);
  __ramfunc_size__ = __ramfunc_end__ - __ramfunc_start__;

  /* 关键代码段 - 用于安全敏感或时间关键的代码 */
  .critical_code :
  {
    . = ALIGN(4);
    __critical_code_start__ = .;
    *(.critical_code)
    *(.critical_code*)
    . = ALIGN(4);
    __critical_code_end__ = .;
  } >RAM AT> FLASH
  __critical_code_load_start__ = LOADADDR(.critical_code);
  __critical_code_size__ = __critical_code_end__ - __critical_code_start__;

  /* 快速访问数据段 - 需要高速访问的数据 */
  .fastdata :
  {
    . = ALIGN(4);
    __fastdata_start__ = .;
    *(.fastdata)
    *(.fastdata*)
    . = ALIGN(4);
    __fastdata_end__ = .;
  } >RAM AT> FLASH
  __fastdata_load_start__ = LOADADDR(.fastdata);
  __fastdata_size__ = __fastdata_end__ - __fastdata_start__;

  /* 关键数据段 - 重要的配置或状态数据 */
  .critical_data :
  {
    . = ALIGN(4);
    __critical_data_start__ = .;
    *(.critical_data)
    *(.critical_data*)
    . = ALIGN(4);
    __critical_data_end__ = .;
  } >RAM AT> FLASH
  __critical_data_load_start__ = LOADADDR(.critical_data);
  __critical_data_size__ = __critical_data_end__ - __critical_data_start__;

  /* ==================== 特殊用途内存段 ==================== */
  
  /* 备份数据段 - 用于掉电保持或特殊用途的数据 */
  .backup (NOLOAD) :
  {
    . = ALIGN(4);
    __backup_start__ = .;
    *(.backup)
    *(.backup*)
    . = ALIGN(4);
    __backup_end__ = .;
  } >RAM
  __backup_size__ = __backup_end__ - __backup_start__;

  /* 共享数据段 - 用于多核或特殊通信的共享内存 */
  .shared (NOLOAD) :
  {
    . = ALIGN(4);
    __shared_start__ = .;
    *(.shared)
    *(.shared*)
    . = ALIGN(4);
    __shared_end__ = .;
  } >RAM
  __shared_size__ = __shared_end__ - __shared_start__;

  /* 引导加载器数据段 - 用于引导加载器通信 */
  .bootloader (NOLOAD) :
  {
    . = ALIGN(4);
    __bootloader_start__ = .;
    *(.bootloader)
    *(.bootloader*)
    . = ALIGN(4);
    __bootloader_end__ = .;
  } >RAM
  __bootloader_size__ = __bootloader_end__ - __bootloader_start__;

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

}


