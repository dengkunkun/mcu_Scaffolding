@echo off
chcp 65001 >nul
echo 串口监控工具启动脚本
echo ========================

REM 检查Python是否安装
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 未找到Python，请先安装Python 3.6+
    pause
    exit /b 1
)

REM 检查是否需要安装依赖
if not exist "%~dp0venv\" (
    echo 首次运行，正在安装依赖...
    python -m pip install -r "%~dp0requirements.txt"
    if %errorlevel% neq 0 (
        echo 警告: 依赖安装可能失败，尝试继续运行...
    )
)

REM 运行串口监控脚本
echo 启动串口监控...
python "%~dp0serial_monitor.py" %*

REM 脚本结束后暂停，方便查看错误信息
if %errorlevel% neq 0 (
    echo.
    echo 脚本执行出现错误，错误代码: %errorlevel%
    pause
)