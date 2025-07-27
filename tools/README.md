# 串口监控工具使用说明

## 功能特点

1. **自动串口检测** - 自动列出所有可用串口供选择
2. **彩色日志输出** - 不同日志级别使用不同颜色显示
3. **实时日志保存** - 自动保存所有串口数据到日志文件
4. **正则表达式过滤** - 支持自定义过滤规则高亮显示
5. **多线程处理** - 数据读取和显示分离，确保实时性

## 安装依赖

```bash
# 进入tools目录
cd tools

# 安装Python依赖
pip install -r requirements.txt
```

## 使用方法

### 1. 基本使用（推荐）

**Windows用户**：
```cmd
# 双击运行或在命令行执行
serial_monitor.bat
```

**Linux/Mac用户**：
```bash
python3 serial_monitor.py
```

### 2. 命令行参数

```bash
# 查看帮助
python serial_monitor.py -h

# 列出所有可用串口
python serial_monitor.py --list

# 指定串口和波特率
python serial_monitor.py -p COM3 -b 115200

# 指定日志文件
python serial_monitor.py -p COM3 -l my_log.txt

# 添加自定义过滤器
python serial_monitor.py -p COM3 -f "temperature.*°C" -f "voltage.*V"
```

### 3. 参数说明

- `-p, --port`: 串口名称（如 COM3, /dev/ttyUSB0）
- `-b, --baudrate`: 波特率（默认115200）
- `-l, --log`: 日志文件路径（默认自动生成）
- `-f, --filter`: 添加正则表达式过滤器
- `--list`: 列出可用串口
- `-t, --timeout`: 超时时间（默认1.0秒）

## 内置过滤器

脚本自动为常见的MCU日志格式添加颜色：

- **红色**: ERROR, ERRO
- **黄色**: WARN, WARNING  
- **绿色**: INFO
- **青色**: DEBUG
- **粗体**: === 标题 ===

## 使用示例

### 示例1：监控STM32日志
```bash
python serial_monitor.py -p COM3 -b 115200
```

### 示例2：过滤特定信息
```bash
python serial_monitor.py -p COM3 -f "Memory.*%" -f "Temperature.*°C"
```

### 示例3：保存到指定日志文件
```bash
python serial_monitor.py -p COM3 -l stm32_debug.log
```

## 输出格式

```
[14:30:25.123] === Application Starting ===
[14:30:25.145] [INFO] System LEDs initialized
[14:30:25.167] [WARN] Some peripherals failed to initialize
[14:30:25.189] [ERROR] Memory allocation failed
```

## 快捷键

- **Ctrl+C**: 停止监控并保存日志
- **终端关闭**: 自动保存日志文件

## 故障排除

### 1. 串口权限问题（Linux）
```bash
sudo usermod -a -G dialout $USER
# 重新登录后生效
```

### 2. Python依赖安装失败
```bash
# 使用pip安装
pip install pyserial

# 或使用conda
conda install pyserial
```

### 3. 串口被占用
- 关闭其他串口工具（如Arduino IDE、PuTTY等）
- 检查是否有其他程序在使用串口

### 4. 中文显示问题（Windows）
- 脚本会自动设置UTF-8编码
- 确保终端支持UTF-8显示

## VS Code集成

可以在VS Code中创建任务来快速启动串口监控：

```json
{
    "label": "Serial Monitor",
    "type": "shell",
    "command": "python",
    "args": ["tools/serial_monitor.py"],
    "group": "build",
    "options": {
        "cwd": "${workspaceFolder}"
    },
    "presentation": {
        "echo": true,
        "reveal": "always",
        "focus": false,
        "panel": "new"
    }
}
```

## 高级功能

### 自定义过滤器示例

```bash
# 监控内存使用情况
python serial_monitor.py -p COM3 -f "Memory.*used.*%"

# 监控温度传感器
python serial_monitor.py -p COM3 -f "Temp.*[0-9]+\.[0-9]+°C"

# 监控网络连接
python serial_monitor.py -p COM3 -f "WiFi.*connected|disconnected"
```

### 日志分析

日志文件格式便于后续分析：
```
[14:30:25.123] [INFO] Application started
[14:30:25.145] [DEBUG] Memory usage: 45.2%
[14:30:25.167] [WARN] Temperature high: 65.4°C
```

可以使用grep、awk等工具进行日志分析：
```bash
# 提取错误信息
grep "ERROR" serial_log_*.txt

# 统计警告数量
grep -c "WARN" serial_log_*.txt
```