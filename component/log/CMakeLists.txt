cmake_minimum_required(VERSION 3.22)

# 引入通用组件配置
include(../ComponentConfig.cmake)

# 设置项目名称
set(LOG_COMPONENT_NAME log)

# 创建静态库目标
add_library(${LOG_COMPONENT_NAME} STATIC)

# 设置源文件
target_sources(${LOG_COMPONENT_NAME} PRIVATE
    # log组件本身的源文件
    src/log.c
    src/elog_port.c
    
    # EasyLogger核心源文件
    ../../ThirdParty/EasyLogger/easylogger/src/elog.c
    ../../ThirdParty/EasyLogger/easylogger/src/elog_utils.c
    ../../ThirdParty/EasyLogger/easylogger/src/elog_buf.c
)

# 设置头文件包含路径
target_include_directories(${LOG_COMPONENT_NAME} PUBLIC
    include 
    src
    ../../ThirdParty/EasyLogger/easylogger/inc
)

# 条件编译选项
option(ELOG_ASYNC_ENABLE "Enable EasyLogger async output" ON)
option(ELOG_FILE_ENABLE "Enable EasyLogger file plugin" OFF)
option(ELOG_FLASH_ENABLE "Enable EasyLogger flash plugin" OFF)

# 异步输出支持
if(ELOG_ASYNC_ENABLE)
    target_sources(${LOG_COMPONENT_NAME} PRIVATE
        ../../ThirdParty/EasyLogger/easylogger/src/elog_async.c
    )
    target_compile_definitions(${LOG_COMPONENT_NAME} PUBLIC
        ELOG_ASYNC_OUTPUT_ENABLE=1
        ELOG_ASYNC_OUTPUT_BUF_SIZE=2048
    )
endif()

# 文件日志插件
if(ELOG_FILE_ENABLE)
    target_sources(${LOG_COMPONENT_NAME} PRIVATE
        ../../ThirdParty/EasyLogger/easylogger/plugins/file/elog_file.c
        ../../ThirdParty/EasyLogger/easylogger/plugins/file/elog_file_port.c
    )
    target_include_directories(${LOG_COMPONENT_NAME} PUBLIC
        ../../ThirdParty/EasyLogger/easylogger/plugins/file
    )
    target_compile_definitions(${LOG_COMPONENT_NAME} PUBLIC ELOG_FILE_ENABLE=1)
endif()

# Flash日志插件
if(ELOG_FLASH_ENABLE)
    target_sources(${LOG_COMPONENT_NAME} PRIVATE
        ../../ThirdParty/EasyLogger/easylogger/plugins/flash/elog_flash.c
        ../../ThirdParty/EasyLogger/easylogger/plugins/flash/elog_flash_port.c
    )
    target_include_directories(${LOG_COMPONENT_NAME} PUBLIC
        ../../ThirdParty/EasyLogger/easylogger/plugins/flash
    )
    target_compile_definitions(${LOG_COMPONENT_NAME} PUBLIC 
        ELOG_FLASH_ENABLE=1
        ELOG_FLASH_BUF_SIZE=1024
    )
endif()

# 获取并链接组件依赖
get_component_dependencies(log LOG_DEPS)
target_link_libraries(${LOG_COMPONENT_NAME} PRIVATE ${LOG_DEPS})

# 链接STM32 HAL库
link_stm32_hal(${LOG_COMPONENT_NAME})

# 设置通用编译选项和属性
set_component_compile_options(${LOG_COMPONENT_NAME})
set_component_properties(${LOG_COMPONENT_NAME} Log)

# 打印配置信息
message(STATUS "Log component configuration:")
message(STATUS "  - EasyLogger async: ${ELOG_ASYNC_ENABLE}")
message(STATUS "  - EasyLogger file: ${ELOG_FILE_ENABLE}")
message(STATUS "  - EasyLogger flash: ${ELOG_FLASH_ENABLE}")