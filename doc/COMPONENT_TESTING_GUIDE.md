# MCU Scaffolding 组件测试系统

本项目提供了完整的组件单元测试架构，基于Unity测试框架，支持在真实硬件上运行测试。

## 📋 测试架构概述

### 测试框架组成
- **Unity测试框架**: 轻量级C语言单元测试框架
- **组件测试**: 每个组件都有独立的测试模块
- **测试构建系统**: 通过CMake选项控制测试/正常固件构建
- **测试运行器**: 在目标硬件上运行所有组件测试

### 目录结构
```
component/
├── worker/
│   ├── test/
│   │   └── test_worker.c         # Worker组件测试
│   └── CMakeLists.txt            # 支持测试模式
├── uart/
│   ├── test/
│   │   └── test_uart.c           # UART组件测试（待添加）
│   └── CMakeLists.txt
└── ...

f103zet6_big/
├── app_main/                     # 正常固件主程序
├── app_main_test/               # 测试固件主程序
│   ├── app_main.c               # 测试运行器
│   └── CMakeLists.txt
└── CMakeLists.txt               # 支持BUILD_TESTS选项

ThirdParty/
└── Unity/                       # Unity测试框架
    ├── src/
    │   ├── unity.c
    │   ├── unity.h
    │   └── unity_config.h        # 嵌入式配置
    └── CMakeLists.txt
```

## 🚀 快速开始

### 1. 构建测试固件

使用专用的测试构建脚本：

```bash
# 构建F103测试固件
./build_test.sh f103

# 清理并重新构建
./build_test.sh f103 -r

# 构建并烧写
./build_test.sh f103 -f

# 查看帮助
./build_test.sh -h
```

### 2. 手动构建测试固件

也可以手动使用CMake构建：

```bash
# 进入项目目录
cd f103zet6_big

# 创建测试构建目录
mkdir build_test && cd build_test

# 配置CMake（启用测试模式）
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..

# 编译
make -j$(nproc)

# 烧写
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "program f103zet6_big_test.hex verify reset exit"
```

### 3. 查看测试结果

连接串口查看测试输出：

```bash
# 使用项目提供的串口监控工具
python tools/serial_monitor.py

# 或使用其他串口工具，波特率115200
```

## 📝 编写组件测试

### Worker组件测试示例

已经实现的Worker组件测试包含：

1. **初始化测试**: 测试Worker线程的创建和初始化
2. **任务提交测试**: 测试基本任务提交和执行
3. **优先级测试**: 测试高优先级任务的优先执行
4. **批量任务测试**: 测试多个任务的并发处理
5. **状态管理测试**: 测试Worker的暂停/恢复功能
6. **队列管理测试**: 测试任务队列的长度管理
7. **清理测试**: 测试Worker的正确销毁

### 为其他组件添加测试

#### 1. 创建测试文件

```c
// component/your_component/test/test_your_component.c
#include "unity.h"
#include "your_component.h"

void setUp(void) {
    // 每个测试前的初始化
}

void tearDown(void) {
    // 每个测试后的清理
}

void test_your_component_function(void) {
    // 测试用例
    TEST_ASSERT_EQUAL_INT(expected, actual);
}

void your_component_test_runner(void) {
    UNITY_BEGIN();
    RUN_TEST(test_your_component_function);
    UNITY_END();
}
```

#### 2. 修改组件CMakeLists.txt

```cmake
# 检查是否启用测试模式
if(ENABLE_COMPONENT_TESTS)
    # 添加测试目标
    add_executable(${COMPONENT_NAME}_test
        test/test_${COMPONENT_NAME}.c
        ${COMPONENT_NAME}.c
    )
    
    # 配置测试目标
    target_link_libraries(${COMPONENT_NAME}_test PRIVATE unity)
    # ... 其他配置
endif()
```

#### 3. 在测试运行器中添加

```c
// f103zet6_big/app_main_test/app_main.c
extern void your_component_test_runner(void);

void test_runner_task(void *argument) {
    // ...existing code...
    
    printf("\n🧪 Running Your Component Tests...\n");
    your_component_test_runner();
    
    // ...existing code...
}
```

## 🎯 测试最佳实践

### 1. 测试命名规范
- 测试文件: `test_component_name.c`
- 测试函数: `test_function_name()`
- 运行器函数: `component_name_test_runner()`

### 2. 测试覆盖范围
- **正常路径**: 测试正常使用场景
- **边界条件**: 测试边界值和极限情况
- **错误处理**: 测试错误输入和异常情况
- **状态转换**: 测试状态机的各种转换
- **资源管理**: 测试内存泄漏和资源释放

### 3. 测试隔离
- 每个测试用例应该相互独立
- 使用`setUp()`和`tearDown()`确保测试环境干净
- 避免测试之间的副作用

### 4. 断言使用
```c
// 基本断言
TEST_ASSERT_TRUE(condition);
TEST_ASSERT_FALSE(condition);
TEST_ASSERT_EQUAL_INT(expected, actual);
TEST_ASSERT_EQUAL_UINT32(expected, actual);
TEST_ASSERT_NOT_NULL(pointer);
TEST_ASSERT_NULL(pointer);

// 字符串断言
TEST_ASSERT_EQUAL_STRING(expected, actual);
TEST_ASSERT_EQUAL_MEMORY(expected, actual, length);
```

## 📊 测试输出示例

```
========================================
  MCU Scaffolding Component Test Suite  
========================================
Build: Dec 25 2024 10:30:45
FreeRTOS Version: V10.4.6
========================================

🧪 Running Worker Component Tests...

=== Worker Component Test Suite ===
Unity test run 1 of 1
test_worker_init_success:PASS
test_worker_init_already_initialized:PASS
test_worker_submit_basic_task:PASS
test_worker_high_priority_task:PASS
test_worker_batch_tasks:PASS
test_worker_state_management:PASS
test_worker_queue_length:PASS
test_worker_destroy:PASS

8 Tests 0 Failures 0 Ignored 
OK

========================================
  All Component Tests Completed!        
========================================
```

## 🔧 配置选项

### CMake选项
- `BUILD_TESTS=ON`: 启用测试构建模式
- `ENABLE_COMPONENT_TESTS=ON`: 启用组件测试
- `CMAKE_BUILD_TYPE=Debug`: 推荐使用Debug模式进行测试

### 编译宏定义
- `APP_TEST_MODE=1`: 应用程序测试模式
- `WORKER_TEST_MODE=1`: Worker组件测试模式
- `UNITY_INCLUDE_CONFIG_H=1`: 包含Unity配置头文件

## 🚨 注意事项

1. **内存限制**: 测试运行在真实硬件上，需要注意内存使用
2. **时序依赖**: 某些测试可能依赖FreeRTOS调度，需要适当的延迟
3. **硬件依赖**: 某些组件测试需要特定的硬件外设支持
4. **串口输出**: 确保串口正确配置，波特率115200

## 📈 扩展计划

- [ ] 添加UART组件测试
- [ ] 添加Memory组件测试
- [ ] 添加Shell组件测试
- [ ] 支持模拟器运行测试
- [ ] 添加性能基准测试
- [ ] 集成到CI/CD流水线

## 🤝 贡献指南

添加新的组件测试时，请确保：
1. 遵循现有的测试架构和命名规范
2. 包含充分的测试覆盖率
3. 更新相关文档
4. 在真实硬件上验证测试通过