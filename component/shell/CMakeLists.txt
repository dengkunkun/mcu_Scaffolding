cmake_minimum_required(VERSION 3.22)

# 引入通用组件配置
include(../ComponentConfig.cmake)

# 设置项目名称
set(SHELL_COMPONENT_NAME shell)

# 创建静态库目标
add_library(${SHELL_COMPONENT_NAME} STATIC)

# 添加第三方lwshell库
add_third_party_library(lwshell ../../ThirdParty/lwshell/lwshell)

# 设置源文件
target_sources(${SHELL_COMPONENT_NAME} PRIVATE
    shell.c
)

# 设置头文件包含路径
target_include_directories(${SHELL_COMPONENT_NAME} PUBLIC
    .                                                    # 当前目录
    ../../ThirdParty/lwshell/lwshell/src                # lwshell头文件
)

# 获取并链接组件依赖
get_component_dependencies(shell SHELL_DEPS)
target_link_libraries(${SHELL_COMPONENT_NAME} PUBLIC
    ${SHELL_DEPS}
)

# 链接STM32 HAL库
link_stm32_hal(${SHELL_COMPONENT_NAME})

# 设置通用编译选项和属性
set_component_compile_options(${SHELL_COMPONENT_NAME})
set_component_properties(${SHELL_COMPONENT_NAME} Shell)