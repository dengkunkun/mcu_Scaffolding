#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
串口监控工具
用于读取串口数据并打印到终端，支持日志记录和数据过滤
"""

import serial
import serial.tools.list_ports
import argparse
import sys
import time
import datetime
import threading
import queue
import os
import re
from typing import Optional, List


class SerialMonitor:
    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 1.0):
        """
        初始化串口监控器
        
        Args:
            port: 串口名称 (如 COM3, /dev/ttyUSB0)
            baudrate: 波特率
            timeout: 超时时间
        """
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial_conn: Optional[serial.Serial] = None
        self.running = False
        self.log_file = None
        self.data_queue = queue.Queue()
        self.filters = []
        
        # 颜色定义 (ANSI escape codes)
        self.colors = {
            'reset': '\033[0m',
            'red': '\033[91m',
            'green': '\033[92m',
            'yellow': '\033[93m',
            'blue': '\033[94m',
            'purple': '\033[95m',
            'cyan': '\033[96m',
            'white': '\033[97m',
            'bold': '\033[1m',
            'dim': '\033[2m'
        }
        
    def list_serial_ports(self) -> List[str]:
        """列出所有可用的串口"""
        ports = serial.tools.list_ports.comports()
        available_ports = []
        
        print(f"{self.colors['cyan']}可用串口列表:{self.colors['reset']}")
        for i, port in enumerate(ports, 1):
            port_info = f"{port.device}"
            if port.description and port.description != "n/a":
                port_info += f" - {port.description}"
            if port.manufacturer:
                port_info += f" ({port.manufacturer})"
                
            print(f"  {i}. {port_info}")
            available_ports.append(port.device)
            
        return available_ports
    
    def connect(self) -> bool:
        """连接串口"""
        try:
            self.serial_conn = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.timeout,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE
            )
            print(f"{self.colors['green']}✓ 串口连接成功: {self.port} @ {self.baudrate} baud{self.colors['reset']}")
            return True
        except serial.SerialException as e:
            print(f"{self.colors['red']}✗ 串口连接失败: {e}{self.colors['reset']}")
            return False
    
    def disconnect(self):
        """断开串口连接"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            print(f"{self.colors['yellow']}串口已断开{self.colors['reset']}")
    
    def setup_logging(self, log_file: str):
        """设置日志文件"""
        try:
            self.log_file = open(log_file, 'a', encoding='utf-8')
            print(f"{self.colors['green']}✓ 日志文件: {log_file}{self.colors['reset']}")
        except Exception as e:
            print(f"{self.colors['red']}✗ 无法创建日志文件: {e}{self.colors['reset']}")
    
    def add_filter(self, pattern: str, highlight_color: str = 'yellow'):
        """添加数据过滤器"""
        try:
            compiled_pattern = re.compile(pattern)
            self.filters.append({
                'pattern': compiled_pattern,
                'color': self.colors.get(highlight_color, self.colors['yellow']),
                'raw_pattern': pattern
            })
            print(f"{self.colors['green']}✓ 添加过滤器: {pattern}{self.colors['reset']}")
        except re.error as e:
            print(f"{self.colors['red']}✗ 无效的正则表达式: {e}{self.colors['reset']}")
    
    def format_timestamp(self) -> str:
        """生成时间戳"""
        return datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
    
    def apply_filters(self, line: str) -> str:
        """应用过滤器高亮显示"""
        formatted_line = line
        for filter_item in self.filters:
            if filter_item['pattern'].search(line):
                # 高亮匹配的内容
                formatted_line = filter_item['pattern'].sub(
                    f"{filter_item['color']}\\g<0>{self.colors['reset']}", 
                    formatted_line
                )
        return formatted_line
    
    def read_data_thread(self):
        """数据读取线程"""
        buffer = b''
        
        while self.running:
            try:
                if self.serial_conn and self.serial_conn.in_waiting > 0:
                    # 读取可用数据
                    data = self.serial_conn.read(self.serial_conn.in_waiting)
                    buffer += data
                    
                    # 按行分割数据
                    while b'\n' in buffer:
                        line_bytes, buffer = buffer.split(b'\n', 1)
                        try:
                            line = line_bytes.decode('utf-8', errors='replace').rstrip('\r')
                            if line:  # 忽略空行
                                self.data_queue.put(line)
                        except Exception as e:
                            print(f"{self.colors['red']}解码错误: {e}{self.colors['reset']}")
                
                time.sleep(0.01)  # 避免CPU占用过高
                
            except serial.SerialException as e:
                print(f"{self.colors['red']}串口读取错误: {e}{self.colors['reset']}")
                break
            except Exception as e:
                print(f"{self.colors['red']}未知错误: {e}{self.colors['reset']}")
                break
    
    def process_data_thread(self):
        """数据处理线程"""
        while self.running:
            try:
                # 从队列获取数据，超时0.1秒
                line = self.data_queue.get(timeout=0.1)
                
                # 生成时间戳
                timestamp = self.format_timestamp()
                
                # 应用过滤器
                formatted_line = self.apply_filters(line)
                
                # 输出到终端
                output = f"{self.colors['dim']}[{timestamp}]{self.colors['reset']} {formatted_line}"
                print(output)
                
                # 写入日志文件
                if self.log_file:
                    log_entry = f"[{timestamp}] {line}\n"
                    self.log_file.write(log_entry)
                    self.log_file.flush()
                
                self.data_queue.task_done()
                
            except queue.Empty:
                continue
            except Exception as e:
                print(f"{self.colors['red']}数据处理错误: {e}{self.colors['reset']}")
    
    def start_monitoring(self):
        """开始监控"""
        if not self.connect():
            return False
            
        self.running = True
        
        # 启动读取和处理线程
        read_thread = threading.Thread(target=self.read_data_thread, daemon=True)
        process_thread = threading.Thread(target=self.process_data_thread, daemon=True)
        
        read_thread.start()
        process_thread.start()
        
        print(f"{self.colors['green']}开始监控串口数据... (按 Ctrl+C 停止){self.colors['reset']}")
        print(f"{self.colors['dim']}================================================{self.colors['reset']}")
        
        try:
            while self.running:
                time.sleep(0.1)
        except KeyboardInterrupt:
            print(f"\n{self.colors['yellow']}用户中断监控{self.colors['reset']}")
        finally:
            self.stop_monitoring()
        
        return True
    
    def stop_monitoring(self):
        """停止监控"""
        self.running = False
        self.disconnect()
        
        if self.log_file:
            self.log_file.close()
            print(f"{self.colors['green']}日志文件已保存{self.colors['reset']}")
    
    def send_data(self, data: str):
        """发送数据到串口"""
        if self.serial_conn and self.serial_conn.is_open:
            try:
                self.serial_conn.write((data + '\n').encode('utf-8'))
                print(f"{self.colors['blue']}发送: {data}{self.colors['reset']}")
            except Exception as e:
                print(f"{self.colors['red']}发送失败: {e}{self.colors['reset']}")


def main():
    parser = argparse.ArgumentParser(description='串口监控工具')
    parser.add_argument('-p', '--port', help='串口名称 (如 COM3, /dev/ttyUSB0)')
    parser.add_argument('-b', '--baudrate', type=int, default=115200, help='波特率 (默认: 115200)')
    parser.add_argument('-l', '--log', help='日志文件路径')
    parser.add_argument('-f', '--filter', action='append', help='数据过滤器 (正则表达式)')
    parser.add_argument('--list', action='store_true', help='列出可用串口')
    parser.add_argument('-t', '--timeout', type=float, default=1.0, help='超时时间 (默认: 1.0秒)')
    
    args = parser.parse_args()
    
    # 列出可用串口
    if args.list:
        monitor = SerialMonitor("", args.baudrate, args.timeout)
        monitor.list_serial_ports()
        return
    
    # 如果没有指定串口，列出可用串口供选择
    if not args.port:
        monitor = SerialMonitor("", args.baudrate, args.timeout)
        available_ports = monitor.list_serial_ports()
        
        if not available_ports:
            print("没有找到可用的串口")
            return
        
        try:
            choice = input(f"\n请选择串口 (1-{len(available_ports)}): ")
            port_index = int(choice) - 1
            if 0 <= port_index < len(available_ports):
                args.port = available_ports[port_index]
            else:
                print("无效的选择")
                return
        except (ValueError, KeyboardInterrupt):
            print("操作取消")
            return
    
    # 创建监控器
    monitor = SerialMonitor(args.port, args.baudrate, args.timeout)
    
    # 设置日志文件
    if args.log:
        monitor.setup_logging(args.log)
    else:
        # 默认日志文件
        log_filename = f"serial_log_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
        monitor.setup_logging(log_filename)
    
    # 添加过滤器
    if args.filter:
        for filter_pattern in args.filter:
            monitor.add_filter(filter_pattern)
    
    # 添加一些常用的MCU日志过滤器
    monitor.add_filter(r'\[ERROR\]|\[ERRO\]|ERROR:', 'red')
    monitor.add_filter(r'\[WARN\]|\[WARNING\]|WARN:', 'yellow')
    monitor.add_filter(r'\[INFO\]|INFO:', 'green')
    monitor.add_filter(r'\[DEBUG\]|DEBUG:', 'cyan')
    monitor.add_filter(r'=== .* ===', 'bold')
    
    # 开始监控
    monitor.start_monitoring()


if __name__ == "__main__":
    main()