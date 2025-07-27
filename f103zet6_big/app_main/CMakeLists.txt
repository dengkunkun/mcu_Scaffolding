cmake_minimum_required(VERSION 3.22)

# 设置项目名称
set(APP_MAIN_COMPONENT_NAME app_main)

# 定义组件路径变量，便于维护
set(COMPONENT_DIR ../../component)

# 定义需要的组件列表
set(REQUIRED_COMPONENTS
    log
    shell
    worker
    memory
    public
)

# 创建静态库目标
add_library(${APP_MAIN_COMPONENT_NAME} STATIC)

# 批量添加组件子目录
foreach(COMPONENT ${REQUIRED_COMPONENTS})
    add_subdirectory(${COMPONENT_DIR}/${COMPONENT} ${COMPONENT}_build)
endforeach()

# 设置源文件
target_sources(${APP_MAIN_COMPONENT_NAME} PRIVATE
    app_main.c
    version_info.c
)

# 设置头文件包含路径
target_include_directories(${APP_MAIN_COMPONENT_NAME} 
    INTERFACE
        include  # 对外公开的头文件目录
    PRIVATE
        .        # 私有头文件目录
        include  # 添加include目录到私有路径
)

# 链接依赖库 - 分层管理依赖
target_link_libraries(${APP_MAIN_COMPONENT_NAME} 
    PRIVATE 
        # 核心组件
        ${REQUIRED_COMPONENTS}
        # STM32 HAL库
        stm32cubemx
)

# 设置编译选项
target_compile_options(${APP_MAIN_COMPONENT_NAME} PRIVATE
    -Wall
    -Wextra
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
)

# 引入通用组件配置
include(${COMPONENT_DIR}/ComponentConfig.cmake)

# 添加版本信息宏（编译时间和Git信息）
add_version_info_macros(${APP_MAIN_COMPONENT_NAME})

# 设置目标属性
set_target_properties(${APP_MAIN_COMPONENT_NAME} PROPERTIES
    EXPORT_NAME AppMain
    OUTPUT_NAME app_main
    DESCRIPTION "Main application component for STM32F103"
)

# 创建别名以便于使用
add_library(AppMain::AppMain ALIAS ${APP_MAIN_COMPONENT_NAME})

# 打印配置信息
message(STATUS "App Main component configuration:")
message(STATUS "  - Required components: ${REQUIRED_COMPONENTS}")
message(STATUS "  - Component directory: ${COMPONENT_DIR}")
message(STATUS "  - Version info macros: enabled")
