cmake_minimum_required(VERSION 3.22)

# 引入通用组件配置
include(../ComponentConfig.cmake)

# 设置项目名称
set(WORKER_COMPONENT_NAME worker)

# 创建静态库目标
add_library(${WORKER_COMPONENT_NAME} STATIC)

# 设置源文件
target_sources(${WORKER_COMPONENT_NAME} PRIVATE
    worker.c
)

# 设置头文件包含路径
target_include_directories(${WORKER_COMPONENT_NAME} 
    PUBLIC 
        include
    PRIVATE
        .
)

# 获取并链接组件依赖
get_component_dependencies(worker WORKER_DEPS)
target_link_libraries(${WORKER_COMPONENT_NAME} PRIVATE ${WORKER_DEPS})

# 链接STM32 HAL库
link_stm32_hal(${WORKER_COMPONENT_NAME})

# 链接FreeRTOS（如果可用）
if(TARGET freertos)
    target_link_libraries(${WORKER_COMPONENT_NAME} PUBLIC freertos)
endif()

# Debug模式的特殊配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${WORKER_COMPONENT_NAME} PRIVATE
        WORKER_DEBUG=1
        WORKER_ENABLE_STATS=1
    )
endif()

# 设置通用编译选项和属性
set_component_compile_options(${WORKER_COMPONENT_NAME})
target_compile_options(${WORKER_COMPONENT_NAME} PRIVATE -Wno-unused-parameter)
set_component_properties(${WORKER_COMPONENT_NAME} Worker)

# 设置额外的目标属性
set_target_properties(${WORKER_COMPONENT_NAME} PROPERTIES
    PUBLIC_HEADER "include/worker.h"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    DESCRIPTION "Linux kernel-style worker thread implementation for FreeRTOS"
    VERSION "1.0.0"
)