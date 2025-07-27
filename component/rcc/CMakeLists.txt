cmake_minimum_required(VERSION 3.22)

# 引入通用组件配置
include(../ComponentConfig.cmake)

# 设置项目名称
set(RCC_COMPONENT_NAME rcc)

# 创建静态库目标
add_library(${RCC_COMPONENT_NAME} STATIC)

# 设置源文件
file(GLOB RCC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
target_sources(${RCC_COMPONENT_NAME} PRIVATE ${RCC_SOURCES})

# 设置头文件包含路径
target_include_directories(${RCC_COMPONENT_NAME} 
    PUBLIC 
        include
    PRIVATE
        .
)

# 获取并链接组件依赖
get_component_dependencies(rcc RCC_DEPS)
if(RCC_DEPS)
    target_link_libraries(${RCC_COMPONENT_NAME} PRIVATE ${RCC_DEPS})
endif()

# 链接STM32 HAL库
link_stm32_hal(${RCC_COMPONENT_NAME})

# 设置通用编译选项和属性
set_component_compile_options(${RCC_COMPONENT_NAME})
set_component_properties(${RCC_COMPONENT_NAME} RCC)

# 设置额外的目标属性
set_target_properties(${RCC_COMPONENT_NAME} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    DESCRIPTION "RCC clock configuration component"
    VERSION "1.0.0"
)