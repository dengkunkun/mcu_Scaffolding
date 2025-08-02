#!/bin/bash
# 通用STM32构建和烧写脚本
# 使用方法：
#   ./build.sh f411              # 构建并烧写F411
#   ./build.sh h743              # 构建并烧写H743
#   ./build.sh f411 -r           # 重新构建F411
#   ./build.sh h743 -r           # 重新构建H743
#   ./build.sh f103 -t           # 构建F103测试固件

# 默认参数
MCU_TYPE=""
REBUILD=false
FLASH_ONLY=false
BUILD_TESTS=false
VERBOSE=false

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        f103|F103)
            MCU_TYPE="f103"
            shift
            ;;
        f411|F411)
            MCU_TYPE="f411"
            shift
            ;;
        h743|H743)
            MCU_TYPE="h743"
            shift
            ;;
        h753|H753)
            MCU_TYPE="h753"
            shift
            ;;
        -r|--rebuild)
            REBUILD=true
            shift
            ;;
        -f|--flash-only)
            FLASH_ONLY=true
            shift
            ;;
        -t|--test)
            BUILD_TESTS=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "使用方法: $0 <mcu_type> [options]"
            echo ""
            echo "MCU类型:"
            echo "  f103    STM32F103ZET6"
            echo "  f411    STM32F411CEU6"
            echo "  h743    STM32H743VIT6"
            echo "  h753    STM32H753IIT6"
            echo ""
            echo "选项:"
            echo "  -r, --rebuild     删除build目录重新构建"
            echo "  -f, --flash-only  仅烧写，不构建"
            echo "  -t, --test        构建测试固件"
            echo "  -v, --verbose     显示详细编译命令"
            echo "  -h, --help        显示此帮助信息"
            echo ""
            echo "示例:"
            echo "  $0 f411           # 构建并烧写F411正常固件"
            echo "  $0 h743 -r        # 重新构建并烧写H743正常固件"
            echo "  $0 f103 -t        # 构建并烧写F103测试固件"
            echo "  $0 f411 -t -r     # 重新构建并烧写F411测试固件"
            echo "  $0 f411 -f        # 仅烧写F411（不构建）"
            echo "  $0 f411 -v        # 显示详细编译命令"
            exit 0
            ;;
        *)
            echo "未知参数: $1"
            echo "使用 $0 --help 查看帮助"
            exit 1
            ;;
    esac
done

# 检查MCU类型参数
if [[ -z "$MCU_TYPE" ]]; then
    echo "错误: 请指定MCU类型 (f103, f411, h743, h753)"
    echo "使用 $0 --help 查看帮助"
    exit 1
fi

# 根据MCU类型设置配置
case $MCU_TYPE in
    f103)
        PROJECT_DIR="f103zet6_big"
        PROJECT_NAME="f103zet6_big"
        OPENOCD_INTERFACE="interface/stlink.cfg"
        OPENOCD_TARGET="target/stm32f1x.cfg"
        ;;
    f411)
        PROJECT_DIR="f411ceu6_nano"
        PROJECT_NAME="f411ceu6_nano"
        OPENOCD_INTERFACE="interface/cmsis-dap.cfg "
        OPENOCD_TARGET="target/stm32f4x.cfg"
        ;;
    h743)
        PROJECT_DIR="h743vit6_mini"
        PROJECT_NAME="h743vit6_mini"
        OPENOCD_INTERFACE="interface/stlink.cfg"
        OPENOCD_TARGET="target/stm32h7x.cfg"
        ;;
    h753)
        PROJECT_DIR="h753_alitek"
        PROJECT_NAME="h753_alitek"
        OPENOCD_INTERFACE="interface/stlink.cfg"
        OPENOCD_TARGET="target/stm32h7x.cfg"
        ;;
esac
if [[ "$BUILD_TESTS" == true ]]; then
    echo "=== 构建测试固件 ==="
    PROJECT_NAME="${PROJECT_NAME}_test"
fi
# 检查项目目录是否存在
if [[ ! -d "$PROJECT_DIR" ]]; then
    echo "错误: 项目目录 $PROJECT_DIR 不存在!"
    exit 1
fi
cd $PROJECT_DIR

# 如果指定重新构建，删除build目录
if [[ "$REBUILD" == true ]]; then
    echo "删除build目录..."
    rm -rf build
fi

# 如果只是烧写，跳过构建
if [[ "$FLASH_ONLY" == true ]]; then
    echo "仅执行烧写，跳过构建..."
    if [[ ! -f "build/${PROJECT_NAME}.hex" ]]; then
        echo "错误: build/${PROJECT_NAME}.hex 不存在!"
        echo "请先构建项目或移除 -f 参数"
        exit 1
    fi
else
    # 创建并进入构建目录
    echo "创建构建目录..."
    mkdir -p build
    cd build

    # 准备CMake参数
    CMAKE_ARGS="-G Ninja"
    if [[ "$BUILD_TESTS" == true ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DBUILD_TESTS=ON"
        echo "🧪 启用测试模式"
    fi
    
    # 如果启用详细模式，添加verbose构建选项
    if [[ "$VERBOSE" == true ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
        echo "🔍 启用详细编译输出"
    fi

    # 使用Ninja构建系统
    echo "配置CMake..."
    echo "CMake命令: cmake $CMAKE_ARGS .."
    cmake $CMAKE_ARGS ..

    # 检查CMake配置是否成功
    if [[ $? -ne 0 ]]; then
        echo "CMake配置失败!"
        exit 1
    fi

    echo "开始构建..."
    if [[ "$VERBOSE" == true ]]; then
        # 使用ninja -v显示详细编译命令
        ninja -v
    else
        ninja
    fi

    # 检查构建是否成功
    if [[ $? -ne 0 ]]; then
        echo "构建失败!"
        exit 1
    fi

    # 生成 HEX 和 BIN 文件
    echo "生成hex和bin文件..."
    
    # 检查实际生成的ELF文件名
    if [[ -f "${PROJECT_NAME}.elf" ]]; then
        ELF_FILE="${PROJECT_NAME}.elf"
    elif [[ -f "${PROJECT_NAME%_test}_test.elf" ]]; then
        ELF_FILE="${PROJECT_NAME%_test}_test.elf"
    elif [[ -f "${PROJECT_NAME}_test.elf" ]]; then
        ELF_FILE="${PROJECT_NAME}_test.elf"
    else
        echo "错误: 找不到ELF文件!"
        echo "查找的文件名: ${PROJECT_NAME}.elf, ${PROJECT_NAME}_test.elf"
        ls -la *.elf 2>/dev/null || echo "构建目录中没有ELF文件"
        exit 1
    fi
    
    echo "使用ELF文件: $ELF_FILE"
    arm-none-eabi-objcopy -O ihex $ELF_FILE ${PROJECT_NAME}.hex
    arm-none-eabi-objcopy -O binary $ELF_FILE ${PROJECT_NAME}.bin

    # 检查文件生成是否成功
    if [[ ! -f "${PROJECT_NAME}.hex" ]]; then
        echo "错误: hex文件生成失败!"
        exit 1
    fi

    # 显示文件大小
    echo ""
    echo "=== 固件信息 ==="
    arm-none-eabi-size $ELF_FILE
    echo ""
    echo "生成的文件:"
    ls -lah $ELF_FILE ${PROJECT_NAME}.hex ${PROJECT_NAME}.bin
    echo ""

    # 返回到项目根目录
    cd ..
fi

# 烧写程序
echo "=== 开始烧写程序 ==="
echo "使用接口: $OPENOCD_INTERFACE"
echo "目标配置: $OPENOCD_TARGET"
echo "烧写文件: build/${PROJECT_NAME}.hex"
echo ""

# 检查hex文件是否存在
if [[ ! -f "build/${PROJECT_NAME}.hex" ]]; then
    echo "错误: build/${PROJECT_NAME}.hex 不存在!"
    exit 1
fi

# 使用OpenOCD烧写
openocd -f $OPENOCD_INTERFACE \
        -f $OPENOCD_TARGET \
        -c "program build/${PROJECT_NAME}.hex verify reset exit"

# 检查烧写结果
if [[ $? -eq 0 ]]; then
    echo ""
    if [[ "$BUILD_TESTS" == true ]]; then
        echo "=== 测试固件烧写成功! ==="
        echo "项目: $PROJECT_NAME (测试版本)"
        echo "芯片: STM32${MCU_TYPE^^}"
        echo ""
        echo "🧪 测试说明:"
        echo "  1. 连接串口 (115200 baud)"
        echo "  2. 重启开发板"
        echo "  3. 观察测试结果输出"
        echo "  4. 所有测试完成后会显示统计信息"
    else
        echo "=== 烧写成功! ==="
        echo "项目: $PROJECT_NAME"
        echo "芯片: STM32${MCU_TYPE^^}"
    fi
else
    echo ""
    echo "=== 烧写失败! ==="
    exit 1
fi

echo ""
echo "=== 调试提示 ==="
echo "启动调试服务器:"
echo "  openocd -f $OPENOCD_INTERFACE -f $OPENOCD_TARGET"
echo ""
echo "连接GDB:"
echo "  arm-none-eabi-gdb build/${PROJECT_NAME}.elf"
echo "  (gdb) target extended-remote localhost:3333"
echo "  (gdb) monitor reset halt"
echo "  (gdb) break main"
echo "  (gdb) continue"