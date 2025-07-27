# Memory Monitor Component
cmake_minimum_required(VERSION 3.22)

# 引入通用组件配置
include(../ComponentConfig.cmake)

# 设置项目名称
set(PUBLIC_COMPONENT_NAME public)

# 创建静态库目标
add_library(${PUBLIC_COMPONENT_NAME} STATIC)

# 设置源文件
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
target_sources(${PUBLIC_COMPONENT_NAME} PRIVATE ${SOURCES})

# 设置头文件包含路径
target_include_directories(${PUBLIC_COMPONENT_NAME} 
    PUBLIC 
        include
        .
    PRIVATE
        .
)

# 获取并链接组件依赖
get_component_dependencies(public PUBLIC_DEPS)
if(PUBLIC_DEPS)
    target_link_libraries(${PUBLIC_COMPONENT_NAME} PRIVATE ${PUBLIC_DEPS})
endif()

# 链接STM32 HAL库
link_stm32_hal(${PUBLIC_COMPONENT_NAME})

# Debug模式的特殊配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PUBLIC_COMPONENT_NAME} PRIVATE
        PUBLIC_DEBUG=1
    )
endif()

# 设置通用编译选项和属性
set_component_compile_options(${PUBLIC_COMPONENT_NAME})
target_compile_options(${PUBLIC_COMPONENT_NAME} PRIVATE -Wno-unused-parameter)
set_component_properties(${PUBLIC_COMPONENT_NAME} Public)

# 设置额外的目标属性
set_target_properties(${PUBLIC_COMPONENT_NAME} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_SOURCE_DIR}"
    DESCRIPTION "Public utilities and common functions"
    VERSION "1.0.0"
)