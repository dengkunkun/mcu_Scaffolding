#!/bin/bash

# 创建并进入构建目录
mkdir -p build
cd build

# 使用Ninja构建系统，避免Makefile路径问题
cmake -G "Ninja" ../f411ceu6_nano
ninja

# 检查构建是否成功
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# 生成 HEX 和 BIN 文件
arm-none-eabi-objcopy -O ihex f411ceu6_nano.elf f411ceu6_nano.hex
arm-none-eabi-objcopy -O binary f411ceu6_nano.elf f411ceu6_nano.bin

# 显示文件大小
arm-none-eabi-size f411ceu6_nano.elf

echo "Programming with OpenOCD..."
# 擦除flash并烧写
# openocd -f interface/cmsis-dap.cfg -f target/stm32f4x.cfg -c "init; reset halt; flash erase_address 0x08000000 0x80000; program f411ceu6_nano.hex verify reset exit"
openocd -f interface/cmsis-dap.cfg -f target/stm32f4x.cfg -c "program f411ceu6_nano.hex verify reset exit"
if [ $? -eq 0 ]; then
    echo "Programming successful!"
else
    echo "Programming failed!"
    exit 1
fi

# cmake -G "Ninja" ../f411ceu6_nano && ninja && arm-none-eabi-objcopy -O ihex f411ceu6_nano.elf f411ceu6_nano.hex && openocd -f interface/cmsis-dap.cfg -f target/stm32f4x.cfg -c "program f411ceu6_nano.hex verify reset exit"