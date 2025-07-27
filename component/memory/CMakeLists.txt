# Memory Monitor Component
cmake_minimum_required(VERSION 3.22)

# 引入通用组件配置
include(../ComponentConfig.cmake)

# 设置项目名称
set(MEMORY_COMPONENT_NAME memory)

# 创建静态库目标
add_library(${MEMORY_COMPONENT_NAME} STATIC)

# 设置源文件
target_sources(${MEMORY_COMPONENT_NAME} PRIVATE
    memory_monitor.c
)

# 设置头文件包含路径
target_include_directories(${MEMORY_COMPONENT_NAME} 
    PUBLIC 
        include
    PRIVATE
        .
)

# 获取并链接组件依赖
get_component_dependencies(memory MEMORY_DEPS)
target_link_libraries(${MEMORY_COMPONENT_NAME} PRIVATE ${MEMORY_DEPS})

# 链接STM32 HAL库
link_stm32_hal(${MEMORY_COMPONENT_NAME})

# Debug模式的特殊配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${MEMORY_COMPONENT_NAME} PRIVATE
        MEMORY_DEBUG=1
    )
endif()

# 设置通用编译选项和属性
set_component_compile_options(${MEMORY_COMPONENT_NAME})
target_compile_options(${MEMORY_COMPONENT_NAME} PRIVATE -Wno-unused-parameter)
set_component_properties(${MEMORY_COMPONENT_NAME} Memory)

# 设置额外的目标属性
set_target_properties(${MEMORY_COMPONENT_NAME} PROPERTIES
    PUBLIC_HEADER "include/memory_monitor.h"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    DESCRIPTION "STM32 Memory monitoring and analysis tool"
    VERSION "1.0.0"
)