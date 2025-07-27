# CMake版本信息宏功能说明

## 概述

在MCU脚手架项目中，我们为CMake构建系统添加了两个重要的版本信息功能：

1. **编译时间宏生成** - 自动生成编译时的时间戳信息
2. **Git信息宏生成** - 自动获取并生成Git分支和提交信息

这些功能通过ComponentConfig.cmake文件中的函数实现，可以在任何组件中轻松使用。

## 新增的CMake函数

### 1. add_build_time_macros(target_name)

自动为指定目标添加编译时间相关的宏定义：

- `BUILD_DATE` - 编译日期 (格式：YYYY-MM-DD)
- `BUILD_TIME` - 编译时间 (格式：HH:MM:SS)
- `BUILD_TIMESTAMP` - 完整时间戳 (格式：YYYY-MM-DD HH:MM:SS UTC)
- `BUILD_YEAR` - 编译年份 (数值)
- `BUILD_MONTH` - 编译月份 (数值)
- `BUILD_DAY` - 编译日期 (数值)

### 2. add_git_info_macros(target_name)

自动为指定目标添加Git相关的宏定义：

- `GIT_BRANCH` - 当前Git分支名
- `GIT_COMMIT_SHORT` - Git提交ID短版本 (7位)
- `GIT_COMMIT_FULL` - Git提交ID完整版本
- `GIT_COMMIT_DATE` - 最后提交的日期
- `GIT_DIRTY` - 工作目录是否有未提交更改 (true/false)

### 3. add_version_info_macros(target_name)

便捷函数，同时添加编译时间和Git信息宏。

## 使用方法

### 在CMakeLists.txt中启用

```cmake
# 引入通用组件配置
include(${COMPONENT_DIR}/ComponentConfig.cmake)

# 为目标添加版本信息宏
add_version_info_macros(your_target_name)

# 或者分别添加
add_build_time_macros(your_target_name)
add_git_info_macros(your_target_name)
```

### 在C代码中使用

```c
#include <stdio.h>

void print_version_info(void)
{
    printf("=== Firmware Version Information ===\n");
    printf("Build Time:    %s\n", BUILD_TIMESTAMP);
    printf("Git Branch:    %s\n", GIT_BRANCH);
    printf("Git Commit:    %s\n", GIT_COMMIT_SHORT);
    printf("Commit Date:   %s\n", GIT_COMMIT_DATE);
    printf("Working Dir:   %s\n", GIT_DIRTY ? "dirty" : "clean");
    printf("===================================\n");
}

// 条件编译示例
#if BUILD_YEAR >= 2025
    // 2025年及以后的特殊处理
#endif
```

## 实际应用示例

在app_main组件中，我们已经集成了完整的版本信息功能，包括：

1. **version_info.h** - 版本信息API声明
2. **version_info.c** - 版本信息API实现
3. **app_main.c** - 在应用启动时自动显示版本信息

运行时输出示例：
```
=== Firmware Version Information ===
Build Time:    2025-01-27 10:30:45 UTC
Git Branch:    feature/version-info
Git Commit:    a1b2c3d
Commit Date:   2025-01-27
Working Dir:   clean
===================================
```

## 故障处理

### Git信息获取失败

如果无法获取Git信息（如不在Git仓库中或Git未安装），函数会：
1. 显示警告信息
2. 使用默认值 "unknown"
3. 设置 GIT_DIRTY 为 false

### 编译时间获取失败

编译时间信息由CMake内置函数生成，一般不会失败。

## 扩展功能

可以根据需要扩展更多版本信息，例如：
- 版本号管理
- 构建配置信息 (Debug/Release)
- 编译器信息
- 硬件平台信息

## 注意事项

1. **每次构建都会更新编译时间** - 这意味着即使源代码没有变化，编译时间戳也会更新
2. **Git信息在构建时获取** - 确保在Git仓库中构建以获取正确信息
3. **跨平台兼容性** - 函数已经处理了Git命令不可用的情况
4. **头文件依赖** - 使用时需要包含 `<stdbool.h>` 支持bool类型

## 应用场景

1. **固件版本追踪** - 便于现场调试和版本管理
2. **自动化测试** - 测试报告中包含详细的构建信息
3. **远程诊断** - 通过日志快速确定固件版本
4. **发布管理** - 确保发布版本的可追溯性