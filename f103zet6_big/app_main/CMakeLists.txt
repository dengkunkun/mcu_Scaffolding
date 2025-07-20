cmake_minimum_required(VERSION 3.22)

# 设置项目名称
set(APP_MAIN_COMPONENT_NAME app_main)

# 创建静态库目标
add_library(${APP_MAIN_COMPONENT_NAME} STATIC)

add_subdirectory(../../component/log log_build
)
add_subdirectory(../../component/shell shell_build
)

# 设置源文件
target_sources(${APP_MAIN_COMPONENT_NAME} PRIVATE
    # 当前目录下的源文件
    app_main.c
)

# 设置头文件包含路径
target_include_directories(${APP_MAIN_COMPONENT_NAME} INTERFACE
    # 对外公开的头文件目录
    include                                                 
)
target_include_directories(${APP_MAIN_COMPONENT_NAME} PRIVATE
   .                                                  
)

target_link_libraries(${APP_MAIN_COMPONENT_NAME} PRIVATE 
    shell   
    log                                         
    stm32cubemx
)

# 设置编译选项
target_compile_options(${APP_MAIN_COMPONENT_NAME} PRIVATE
    -Wall
    -Wextra
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
)
