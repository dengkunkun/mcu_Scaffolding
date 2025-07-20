# freertos

gdb默认情况下无法完整的使用info threads功能，借助 https://github.com/espressif/freertos-gdb 可以友好的打印
freertos-gdb依赖：
    支持python的gdb，只有linux的gdb支持，否则需要自行编译，下面是在wsl下
    https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads/  #最新版不支持，不知道为什么
    https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads/14-2-rel1  #linux下支持
    python3.8
        sudo add-apt-repository ppa:deadsnakes/ppa
        sudo apt update
        sudo apt install libpython3.8
        sudo apt install build-essential libncurses5 libncursesw5 libtinfo5 zlib1g
    pip install freertos-gdb

    
    (gdb) python
    import sys
    sys.path.append('/home/dengkunkun/.local/lib/python3.10/site-packages')
    import freertos_gdb
    end
   
    #py结尾的gdb
    arm-none-eabi-gdb-py  build/f103zet6_big.elf -ex "target extended-remote localhost:3333" -ex "monitor reset halt" -ex "load" -ex "monitor reset halt" 

使用.gdbinit自动加载
写入~/.gdbinit  
python
import sys
sys.path.append('/home/dengkunkun/.local/lib/python3.10/site-packages')
import freertos_gdb
end

写入执行gdb的位置可能报错如下，按提示执行即可
warning: File "/mnt/c/Users/kunkun/Desktop/mcu_Scaffolding/f103zet6_big/.gdbinit" auto-loading has been declined by your `auto-load safe-path' set to "$debugdir:$datadir/auto-load".
To enable execution of this file add
        add-auto-load-safe-path /mnt/c/Users/kunkun/Desktop/mcu_Scaffolding/f103zet6_big/.gdbinit
line to your configuration file "/home/dengkunkun/.config/gdb/gdbinit".
To completely disable this security protection add
        set auto-load safe-path /
line to your configuration file "/home/dengkunkun/.config/gdb/gdbinit".


仍然无法使用thread切换线程，需要继续探索


# 调试
客户端：
arm-none-eabi-gdb build/f103zet6_big.elf -ex "target extended-remote localhost:3333" -ex "monitor reset halt" -ex "load" -ex "monitor reset halt"

服务端：
openocd -f openocd.cfg

openocd.cfg:
~~~txt
# OpenOCD configuration for STM32F103 with FreeRTOS support
source [find interface/stlink.cfg]
source [find target/stm32f1x.cfg]

# Enable FreeRTOS thread awareness
$_TARGETNAME configure -rtos FreeRTOS

# Optional: Set adapter speed
adapter speed 1000

# Initialize
init
~~~

# vscode配置

# OpenOCD + GDB 命令行调试完整流程

## 1. 启动调试服务器
```bash
# 方法1：使用项目配置文件
cd f103zet6_big
openocd -f openocd.cfg

# 方法2：直接指定配置
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# 输出示例：
# Info : Listening on port 3333 for gdb connections
# Info : Listening on port 6666 for tcl connections  
# Info : Listening on port 4444 for telnet connections
```

## 2. 连接GDB客户端
```bash
# 启动GDB并连接
arm-none-eabi-gdb build/f103zet6_big.elf \
    -ex "target extended-remote localhost:3333" \
    -ex "monitor reset halt" \
    -ex "load" \
    -ex "monitor reset halt"

# 或者分步执行
arm-none-eabi-gdb build/f103zet6_big.elf
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) monitor reset halt
```

## 3. 常用调试命令

### 程序控制
```gdb
(gdb) break main               # 在main设置断点
(gdb) continue                 # 继续执行
(gdb) step                     # 单步进入函数
(gdb) next                     # 单步不进入函数
(gdb) finish                   # 执行到函数返回
(gdb) until                    # 执行到下一行
```

### 断点管理
```gdb
(gdb) break filename.c:123     # 在指定文件行设置断点
(gdb) break function_name      # 在函数设置断点
(gdb) break *0x08000100        # 在地址设置断点
(gdb) info breakpoints         # 查看所有断点
(gdb) delete 1                 # 删除断点号1
(gdb) disable 1                # 禁用断点号1
(gdb) enable 1                 # 启用断点号1
```

### 变量和内存查看
```gdb
(gdb) print variable_name      # 打印变量值
(gdb) print/x variable_name    # 以16进制打印
(gdb) print *pointer           # 打印指针指向的值
(gdb) display variable_name    # 自动显示变量
(gdb) x/10x 0x20000000        # 查看内存（16进制）
(gdb) x/10i $pc               # 查看指令
(gdb) info registers          # 查看所有寄存器
```

### FreeRTOS 多线程调试
```gdb
(gdb) info threads            # 查看所有线程
(gdb) thread 2                # 切换到线程2
(gdb) thread apply all bt     # 显示所有线程的调用栈
(gdb) p pxCurrentTCB->pcTaskName  # 查看当前任务名
(gdb) p uxCurrentNumberOfTasks    # 查看任务数量
```

## 4. OpenOCD 监控命令

可以通过GDB的monitor命令或telnet连接到OpenOCD：

### 通过GDB监控
```gdb
(gdb) monitor reset halt      # 复位并停止
(gdb) monitor reset init      # 复位并初始化
(gdb) monitor reg            # 查看寄存器
(gdb) monitor mdw 0x20000000 # 读取内存字
```

### 通过Telnet连接
```bash
telnet localhost 4444

# 在telnet中可以执行：
> reset halt
> reg
> mdw 0x20000000
> flash info 0
```

## 5. 烧写相关命令

### 完整烧写流程
```bash
# 一条命令完成烧写
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "program build/f103zet6_big.hex verify reset exit"

# 分步烧写
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "init; reset halt; flash write_image erase build/f103zet6_big.hex; verify_image build/f103zet6_big.hex; reset run; exit"
```

### 擦除操作
```bash
# 擦除整个芯片
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "init; reset halt; stm32f1x mass_erase 0; exit"

# 擦除指定扇区
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "init; reset halt; flash erase_sector 0 0 10; exit"
```
我来详细介绍如何使用OpenOCD进行STM32的烧写和调试，以及VS Code的配置方法。

## OpenOCD 基本概念

OpenOCD (Open On-Chip Debugger) 是一个开源的在线调试器，支持多种调试接口和目标芯片。它作为GDB和硬件调试器之间的桥梁。

## 1. OpenOCD 烧写操作

### 基本烧写命令

```bash
# 基本烧写格式
openocd -f <interface_config> -f <target_config> -c "<commands>"

# 示例：烧写STM32F103
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "program build/f103zet6_big.hex verify reset exit"
```

### 常用烧写命令

```bash
# 1. 烧写并验证
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "program firmware.hex verify reset exit"

# 2. 烧写ELF文件
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "program firmware.elf verify reset exit"

# 3. 烧写到指定地址
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "program firmware.bin 0x08000000 verify reset exit"

# 4. 擦除芯片
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "init; reset halt; stm32f1x mass_erase 0; exit"
```

### 你的build.sh脚本分析

从你的build.sh可以看到，你已经实现了自动化的构建和烧写：

```bash
# 构建项目
./build.sh f103           # 构建并烧写F103
./build.sh f411 -r        # 重新构建F411
./build.sh h743 -f        # 仅烧写H743（不构建）
```

## 2. OpenOCD 调试配置

### 启动调试服务器

```bash
# 基本调试服务器启动
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# 使用你的配置文件
openocd -f openocd.cfg
```

### 你的openocd.cfg配置解析

```tcl
# OpenOCD configuration for STM32F103 with FreeRTOS support
source [find interface/stlink.cfg]     # ST-Link调试器配置
source [find target/stm32f1x.cfg]      # STM32F1系列目标配置

# Enable FreeRTOS thread awareness
$_TARGETNAME configure -rtos FreeRTOS  # 启用FreeRTOS线程感知

# Optional: Set adapter speed
adapter speed 1000                     # 设置调试速度

# Initialize
init                                   # 初始化
```

## 3. GDB 调试常用命令

### 连接和基本操作

```gdb
# 连接到OpenOCD
(gdb) target extended-remote localhost:3333

# 复位和停止
(gdb) monitor reset halt

# 加载程序
(gdb) load

# 运行控制
(gdb) continue          # 继续执行
(gdb) step             # 单步进入
(gdb) next             # 单步跳过
(gdb) finish           # 执行到函数返回

# 断点操作
(gdb) break main       # 在main函数设置断点
(gdb) break *0x08000100 # 在地址设置断点
(gdb) info breakpoints # 查看断点
(gdb) delete 1         # 删除断点1
```

### FreeRTOS 调试命令

```gdb
# 查看所有线程
(gdb) info threads

# 切换线程
(gdb) thread 2

# 查看线程调用栈
(gdb) backtrace
(gdb) bt

# 查看所有线程的调用栈
(gdb) thread apply all bt

# FreeRTOS特定信息
(gdb) p uxCurrentNumberOfTasks
(gdb) p pxCurrentTCB->pcTaskName
```

### 内存和寄存器操作

```gdb
# 查看内存
(gdb) x/10x 0x20000000  # 查看内存（16进制）
(gdb) x/10i $pc         # 查看指令

# 查看寄存器
(gdb) info registers
(gdb) p $pc
(gdb) p $sp

# 查看变量
(gdb) print variable_name
(gdb) display variable_name  # 自动显示
```

## 4. VS Code 调试配置详解

### launch.json 配置分析

你的launch.json包含了多种调试配置，我来详细解释：

#### 基本调试配置（STM32F103）

```json
{
    "name": "Debug STM32F103 (ST-Link)",
    "cwd": "${workspaceFolder}/f103zet6_big",
    "executable": "${workspaceFolder}/f103zet6_big/build/f103zet6_big.elf",
    "request": "launch",
    "type": "cortex-debug",
    "runToEntryPoint": "main",
    "showDevDebugOutput": "parsed",
    "servertype": "openocd",
    "configFiles": [
        "interface/stlink.cfg",
        "target/stm32f1x.cfg"
    ],
    "preLaunchTask": "Build F103",          # 调试前先构建
    "postLaunchCommands": [                 # 调试启动后执行的命令
        "monitor reset halt",
        "break main",
        "continue"
    ]
}
```

#### 连接运行中的程序（Attach模式）

```json
{
    "name": "Attach STM32F103 (Running)",
    "request": "attach",                    # 连接模式，不重新烧写
    "type": "cortex-debug",
    // ... 其他配置
}
```

### VS Code 扩展推荐

需要安装以下扩展：

1. **Cortex-Debug** - ARM Cortex调试支持
2. **C/C++** - Microsoft的C/C++支持
3. **CMake Tools** - CMake项目支持

## 5. 完整的调试工作流

### 方法1：使用VS Code调试

1. **按F5启动调试**
   - VS Code会自动执行preLaunchTask（构建项目）
   - 启动OpenOCD服务器
   - 连接GDB并加载程序

2. **调试操作**
   - 设置断点：点击行号左侧
   - 单步调试：F10（step over）, F11（step into）
   - 查看变量：鼠标悬停或在Watch窗口添加
   - 查看寄存器：在REGISTERS面板

### 方法2：命令行调试
````
# OpenOCD + GDB 命令行调试完整流程

## 1. 启动调试服务器
```bash
# 方法1：使用项目配置文件
cd f103zet6_big
openocd -f openocd.cfg

# 方法2：直接指定配置
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# 输出示例：
# Info : Listening on port 3333 for gdb connections
# Info : Listening on port 6666 for tcl connections  
# Info : Listening on port 4444 for telnet connections
```

## 2. 连接GDB客户端
```bash
# 启动GDB并连接
arm-none-eabi-gdb build/f103zet6_big.elf \
    -ex "target extended-remote localhost:3333" \
    -ex "monitor reset halt" \
    -ex "load" \
    -ex "monitor reset halt"

# 或者分步执行
arm-none-eabi-gdb build/f103zet6_big.elf
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) monitor reset halt
```

## 3. 常用调试命令

### 程序控制
```gdb
(gdb) break main               # 在main设置断点
(gdb) continue                 # 继续执行
(gdb) step                     # 单步进入函数
(gdb) next                     # 单步不进入函数
(gdb) finish                   # 执行到函数返回
(gdb) until                    # 执行到下一行
```

### 断点管理
```gdb
(gdb) break filename.c:123     # 在指定文件行设置断点
(gdb) break function_name      # 在函数设置断点
(gdb) break *0x08000100        # 在地址设置断点
(gdb) info breakpoints         # 查看所有断点
(gdb) delete 1                 # 删除断点号1
(gdb) disable 1                # 禁用断点号1
(gdb) enable 1                 # 启用断点号1
```

### 变量和内存查看
```gdb
(gdb) print variable_name      # 打印变量值
(gdb) print/x variable_name    # 以16进制打印
(gdb) print *pointer           # 打印指针指向的值
(gdb) display variable_name    # 自动显示变量
(gdb) x/10x 0x20000000        # 查看内存（16进制）
(gdb) x/10i $pc               # 查看指令
(gdb) info registers          # 查看所有寄存器
```

### FreeRTOS 多线程调试
```gdb
(gdb) info threads            # 查看所有线程
(gdb) thread 2                # 切换到线程2
(gdb) thread apply all bt     # 显示所有线程的调用栈
(gdb) p pxCurrentTCB->pcTaskName  # 查看当前任务名
(gdb) p uxCurrentNumberOfTasks    # 查看任务数量
```

## 4. OpenOCD 监控命令

可以通过GDB的monitor命令或telnet连接到OpenOCD：

### 通过GDB监控
```gdb
(gdb) monitor reset halt      # 复位并停止
(gdb) monitor reset init      # 复位并初始化
(gdb) monitor reg            # 查看寄存器
(gdb) monitor mdw 0x20000000 # 读取内存字
```

### 通过Telnet连接
```bash
telnet localhost 4444

# 在telnet中可以执行：
> reset halt
> reg
> mdw 0x20000000
> flash info 0
```

## 5. 烧写相关命令

### 完整烧写流程
```bash
# 一条命令完成烧写
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "program build/f103zet6_big.hex verify reset exit"

# 分步烧写
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "init; reset halt; flash write_image erase build/f103zet6_big.hex; verify_image build/f103zet6_big.hex; reset run; exit"
```

### 擦除操作
```bash
# 擦除整个芯片
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "init; reset halt; stm32f1x mass_erase 0; exit"

# 擦除指定扇区
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "init; reset halt; flash erase_sector 0 0 10; exit"
```

...existing code...
````


Made changes.

继续补充调试配置和使用方法：

## 6. VS Code 调试面板功能详解

### 调试面板区域
- **VARIABLES** - 查看当前作用域的变量
- **WATCH** - 添加需要监视的表达式
- **CALL STACK** - 显示函数调用栈
- **BREAKPOINTS** - 管理所有断点
- **REGISTERS** - 查看CPU寄存器状态
- **MEMORY** - 内存查看器
- **DISASSEMBLY** - 反汇编视图

### 调试快捷键
```
F5     - 启动调试/继续执行
F9     - 切换断点
F10    - 单步跳过 (Step Over)
F11    - 单步进入 (Step Into)
Shift+F11 - 单步退出 (Step Out)
Ctrl+Shift+F5 - 重启调试
Shift+F5 - 停止调试
```

## 7. 高级调试技巧

### 条件断点
```gdb
# 在GDB中设置条件断点
(gdb) break main if argc > 1

# 在VS Code中：右键断点 -> Edit Breakpoint -> 添加条件
```

### 日志断点（Logpoints）
在VS Code中，右键行号选择"Add Logpoint"，可以在不停止程序的情况下输出信息。

### 内存监视
```gdb
# 监视内存范围
(gdb) watch *(int*)0x20000000
(gdb) rwatch variable_name    # 读取监视
(gdb) awatch variable_name    # 访问监视
```

## 8. 多项目调试配置

你的launch.json已经很好地配置了多个STM32项目的调试，每个配置的关键区别：

### 接口配置差异
- **F103/H743/H753**: 使用ST-Link (`interface/stlink.cfg`)
- **F411**: 使用CMSIS-DAP (`interface/cmsis-dap.cfg`)

### 目标配置差异
- **F103**: `target/stm32f1x.cfg`
- **F411**: `target/stm32f4x.cfg`  
- **H743/H753**: `target/stm32h7x.cfg`

## 9. 常见问题解决

### OpenOCD连接失败
```bash
# 检查设备连接
lsusb | grep -i stlink

# 权限问题（Linux）
sudo openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# 端口被占用
sudo netstat -tulpn | grep :3333
```

### GDB连接问题
```gdb
# 检查OpenOCD是否启动
telnet localhost 3333

# 重新连接
(gdb) disconnect
(gdb) target extended-remote localhost:3333
```

### 烧写失败
```bash
# 检查芯片保护
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "init; reset halt; stm32f1x options_read 0; exit"

# 解除保护
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "init; reset halt; stm32f1x unlock 0; exit"
```

## 10. 性能调试

### 使用SWO (Serial Wire Output)
```gdb
# 在OpenOCD配置中启用SWO
tpiu config internal swotrace.log uart off 72000000

# 在代码中使用ITM输出
ITM_SendChar('H');
```

### FreeRTOS任务监控
```gdb
# 查看任务状态
(gdb) p pxCurrentTCB
(gdb) p *pxCurrentTCB

# 查看任务堆栈使用情况
(gdb) p uxTaskGetStackHighWaterMark(task_handle)
```

## 11. 自动化调试脚本

### GDB初始化脚本
创建 `.gdbinit` 文件：
```gdb
# 连接到目标
target extended-remote localhost:3333

# 自定义命令
define reset_and_load
    monitor reset halt
    load
    monitor reset halt
end

# 自动断点
break main
break HardFault_Handler

# 显示设置
set print pretty on
set pagination off
```

### VS Code任务集成
你的build.sh已经很好地集成了构建、烧写流程，VS Code的preLaunchTask会在调试前自动执行构建。

这样的配置让你可以：
1. 一键调试（F5）
2. 自动构建最新代码
3. 支持多个STM32项目
4. 完整的调试功能支持

通过这套配置，你可以高效地进行STM32的开发和调试工作。



我来详细介绍VS Code插件RTOS VIEW的使用方法。RTOS VIEW是一个专门用于实时操作系统调试的强大插件，可以显示任务状态、内存使用情况、队列等信息。

## 1. RTOS VIEW 插件简介

RTOS VIEW 是一个VS Code扩展，专门用于调试和监控实时操作系统（如FreeRTOS、ThreadX、RT-Thread等）的运行状态。它提供了图形化界面来查看：

- **任务/线程状态**
- **内存使用情况** 
- **队列和信号量状态**
- **定时器信息**
- **系统统计信息**

## 2. 安装配置

### 安装插件

在VS Code扩展市场搜索安装：
```
RTOS Views
```

或者安装相关的调试插件组合：
- **Cortex-Debug** (必需)
- **RTOS Views** 
- **RTOS Thread Viewer**

### 配置launch.json

需要在你的调试配置中启用RTOS支持：
````
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug STM32F103 (ST-Link)",
            // ...existing code...
            "servertype": "openocd",
            "configFiles": [
                "interface/stlink.cfg",
                "target/stm32f1x.cfg"
            ],
            // 添加RTOS支持配置
            "rtos": "FreeRTOS",                    // 指定RTOS类型
            "showDevDebugOutput": "parsed",
            "swoConfig": {
                "enabled": true,
                "cpuFrequency": 72000000,
                "swoFrequency": 2000000,
                "source": "probe",
                "decoders": [
                    {
                        "type": "console",
                        "label": "ITM",
                        "port": 0
                    }
                ]
            },
            // ...existing code...
        },
        {
            "name": "Debug STM32F411 (CMSIS-DAP)",
            // ...existing code...
            "rtos": "FreeRTOS",
            // ...existing code...
        },
        {
            "name": "Debug STM32H743 (ST-Link)",
            // ...existing code...
            "rtos": "FreeRTOS",
            // ...existing code...
        },
        {
            "name": "Debug STM32H753 (ST-Link)",
            // ...existing code...
            "rtos": "FreeRTOS",
            // ...existing code...
        }
        // ...existing code...
    ]
}
````


Made changes.

现在继续介绍RTOS VIEW的使用方法：

## 3. 代码配置要求

### FreeRTOS配置

确保你的FreeRTOS配置支持调试功能，在`FreeRTOSConfig.h`中需要这些配置：

```c
#define configUSE_TRACE_FACILITY                 1
#define configGENERATE_RUN_TIME_STATS            1
#define configUSE_STATS_FORMATTING_FUNCTIONS     1
#define INCLUDE_uxTaskGetStackHighWaterMark      1
#define INCLUDE_xTaskGetCurrentTaskHandle        1
#define INCLUDE_eTaskGetState                    1
#define INCLUDE_xTaskGetIdleTaskHandle           1
#define INCLUDE_pxTaskGetStackStart              1
```

## 4. 使用RTOS VIEW调试

### 启动调试会话

1. **按F5或选择调试配置**启动调试
2. **等待程序运行到断点**（如main函数）
3. **在VS Code侧边栏**会出现RTOS相关面板

### RTOS VIEW面板功能

调试启动后，你会看到以下面板：

#### 4.1 RTOS Tasks/Threads 面板
显示所有FreeRTOS任务信息：
- **任务名称**
- **状态**（Running, Ready, Blocked, Suspended, Deleted）
- **优先级**
- **堆栈使用情况**
- **堆栈剩余空间**

#### 4.2 RTOS Queues 面板
显示队列和信号量状态：
- **队列名称**
- **当前消息数量**
- **最大容量**
- **等待发送的任务**
- **等待接收的任务**

#### 4.3 RTOS Timers 面板
显示软件定时器信息：
- **定时器名称**
- **状态**（Active/Inactive）
- **周期**
- **剩余时间**

#### 4.4 Memory Usage 面板
显示内存使用统计：
- **堆内存使用情况**
- **各任务堆栈使用情况**
- **内存碎片信息**

## 5. 实际使用示例

### 测试RTOS VIEW功能

让我们先构建F103项目来演示：看起来F103项目构建失败了。让我继续介绍RTOS VIEW的使用方法，并提供一些实用的调试技巧。

## 5. RTOS VIEW 实际操作指南

### 5.1 查看任务状态

当你启动调试会话后，RTOS VIEW会显示类似这样的任务信息：

```
RTOS Tasks:
┌─────────────┬─────────┬──────────┬──────────┬─────────────┐
│ Task Name   │ State   │ Priority │ Stack    │ Handle      │
├─────────────┼─────────┼──────────┼──────────┼─────────────┤
│ IDLE        │ Ready   │ 0        │ 85/128   │ 0x20001234  │
│ app_main    │ Running │ 24       │ 156/512  │ 0x20001456  │
│ log_task    │ Blocked │ 10       │ 98/256   │ 0x20001678  │
└─────────────┴─────────┴──────────┴──────────┴─────────────┘
```

### 5.2 切换任务上下文调试

在RTOS Tasks面板中：
1. **双击任务名称**可以切换到该任务的上下文
2. **调用栈会自动更新**显示该任务的函数调用链
3. **变量窗口显示**该任务相关的局部变量

### 5.3 分析堆栈使用情况

RTOS VIEW帮助你监控堆栈溢出：
- **绿色**：堆栈使用正常（< 75%）
- **黄色**：堆栈使用较高（75-90%）
- **红色**：堆栈使用危险（> 90%）

## 6. 高级调试技巧

### 6.1 实时任务监控

添加自定义监视表达式：
```c
// 在Watch面板添加这些表达式
pxCurrentTCB->pcTaskName          // 当前任务名
uxTaskGetNumberOfTasks()          // 任务总数
xPortGetFreeHeapSize()            // 剩余堆内存
uxTaskGetStackHighWaterMark(NULL) // 当前任务堆栈水位
```

### 6.2 SWO 跟踪输出

在launch.json中已配置SWO，可以在代码中使用：

```c
#include "core_cm3.h"  // 或对应的核心头文件

// 在代码中添加跟踪输出
void debug_printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    for(int i = 0; buffer[i]; i++) {
        ITM_SendChar(buffer[i]);
    }
}

// 使用示例
debug_printf("Task %s started\n", pcTaskGetName(NULL));
```

### 6.3 任务运行时统计

启用运行时统计后，可以查看：
```c
// 获取任务运行时间统计
char statsBuffer[1024];
vTaskGetRunTimeStats(statsBuffer);
// 在调试器中查看 statsBuffer 内容
```

## 7. 常见问题解决

### 7.1 RTOS VIEW 不显示任务

**可能原因：**
1. FreeRTOS配置不正确
2. 调度器尚未启动
3. 符号信息缺失

**解决方法：**
```c
// 确保这些配置在 FreeRTOSConfig.h 中
#define configUSE_TRACE_FACILITY    1
#define INCLUDE_uxTaskGetStackHighWaterMark  1

// 在main函数中确认调度器已启动
if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {
    // 调度器未运行
}
```

### 7.2 堆栈信息显示不准确

在CMakeLists.txt中确保调试信息完整：
```cmake
set(CMAKE_C_FLAGS_DEBUG "-g3 -Og")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -Og")
```

### 7.3 任务切换时断点失效

这是正常现象，可以使用：
```gdb
# 在所有任务中设置断点
(gdb) thread apply all break function_name

# 或设置条件断点
(gdb) break function_name if strcmp(pcTaskGetName(NULL), "app_main") == 0
```

## 8. 与你的项目集成

基于你的多MCU项目结构，RTOS VIEW的配置已经添加到launch.json中。现在你可以：

1. **选择对应的调试配置**（F103/F411/H743/H753）
2. **按F5启动调试**
3. **在RTOS面板查看**任务状态、队列、定时器等信息
4. 
