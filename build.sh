#!/bin/bash
# 通用STM32构建和烧写脚本
# 使用方法：
#   ./build.sh f411              # 构建并烧写F411
#   ./build.sh h743              # 构建并烧写H743
#   ./build.sh f411 -r           # 重新构建F411
#   ./build.sh h743 -r           # 重新构建H743

# 默认参数
MCU_TYPE=""
REBUILD=false
FLASH_ONLY=false

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
        -h|--help)
            echo "使用方法: $0 <mcu_type> [options]"
            echo ""
            echo "MCU类型:"
            echo "  f411    STM32F411CEU6"
            echo "  h743    STM32H743VIT6"
            echo ""
            echo "选项:"
            echo "  -r, --rebuild     删除build目录重新构建"
            echo "  -f, --flash-only  仅烧写，不构建"
            echo "  -h, --help        显示此帮助信息"
            echo ""
            echo "示例:"
            echo "  $0 f411           # 构建并烧写F411"
            echo "  $0 h743 -r        # 重新构建并烧写H743"
            echo "  $0 f411 -f        # 仅烧写F411（不构建）"
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
    echo "错误: 请指定MCU类型 (f411 或 h743)"
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
        echo "=== 构建STM32F103ZET6项目 ==="
        ;;
    f411)
        PROJECT_DIR="f411ceu6_nano"
        PROJECT_NAME="f411ceu6_nano"
        OPENOCD_INTERFACE="interface/cmsis-dap.cfg "
        OPENOCD_TARGET="target/stm32f4x.cfg"
        echo "=== 构建STM32F411CEU6项目 ==="
        ;;
    h743)
        PROJECT_DIR="h743vit6_mini"
        PROJECT_NAME="h743vit6_mini"
        OPENOCD_INTERFACE="interface/stlink.cfg"
        OPENOCD_TARGET="target/stm32h7x.cfg"
        echo "=== 构建STM32H743VIT6项目 ==="
        ;;
    h753)
        PROJECT_DIR="h753_alitek"
        PROJECT_NAME="h753_alitek"
        OPENOCD_INTERFACE="interface/stlink.cfg"
        OPENOCD_TARGET="target/stm32h7x.cfg"
        echo "=== 构建STM32H753IIT6项目 ==="
        ;;
esac

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

    # 使用Ninja构建系统
    echo "配置CMake..."
    cmake -G "Ninja" ..

    # 检查CMake配置是否成功
    if [[ $? -ne 0 ]]; then
        echo "CMake配置失败!"
        exit 1
    fi

    echo "开始构建..."
    ninja

    # 检查构建是否成功
    if [[ $? -ne 0 ]]; then
        echo "构建失败!"
        exit 1
    fi

    # 生成 HEX 和 BIN 文件
    echo "生成hex和bin文件..."
    arm-none-eabi-objcopy -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    arm-none-eabi-objcopy -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin

    # 检查文件生成是否成功
    if [[ ! -f "${PROJECT_NAME}.hex" ]]; then
        echo "错误: hex文件生成失败!"
        exit 1
    fi

    # 显示文件大小
    echo ""
    echo "=== 固件信息 ==="
    arm-none-eabi-size ${PROJECT_NAME}.elf
    echo ""
    echo "生成的文件:"
    ls -lah ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex ${PROJECT_NAME}.bin
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
    echo "=== 烧写成功! ==="
    echo "项目: $PROJECT_NAME"
    echo "芯片: STM32${MCU_TYPE^^}"
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