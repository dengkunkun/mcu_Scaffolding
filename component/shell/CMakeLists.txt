cmake_minimum_required(VERSION 3.22)

# 设置项目名称
set(SHELL_COMPONENT_NAME shell)

# 创建静态库目标
add_library(${SHELL_COMPONENT_NAME} STATIC)

# 修正：为out-of-tree源目录指定二进制目录
add_subdirectory(../../ThirtParty/lwshell/lwshell ${CMAKE_CURRENT_BINARY_DIR}/lwshell)

# 设置源文件
target_sources(${SHELL_COMPONENT_NAME} PRIVATE
    # 当前目录下的源文件
    shell.c
)

# 设置头文件包含路径
target_include_directories(${SHELL_COMPONENT_NAME} PUBLIC
    # 对外公开的头文件目录
    .                                                    # 当前目录
    ../../ThirtParty/lwshell/lwshell/src                # lwshell头文件
)

# 链接lwshell库
target_link_libraries(${SHELL_COMPONENT_NAME} PRIVATE 
    lwshell    
    log                                         # 链接lwshell库
    stm32cubemx
)

# 设置编译选项
target_compile_options(${SHELL_COMPONENT_NAME} PRIVATE
    -Wall
    -Wextra
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
)

# 导出目标
set_target_properties(${SHELL_COMPONENT_NAME} PROPERTIES
    EXPORT_NAME Shell
    OUTPUT_NAME shell
)

# 创建别名
add_library(Shell::Shell ALIAS ${SHELL_COMPONENT_NAME})