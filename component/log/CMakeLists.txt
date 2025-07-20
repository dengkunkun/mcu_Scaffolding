cmake_minimum_required(VERSION 3.22)

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

    
    # 根据配置添加异步和缓冲模块（可选）
    # ../../ThirdParty/EasyLogger/easylogger/src/elog_async.c
    # ../../ThirdParty/EasyLogger/easylogger/src/elog_buf.c
)

# 设置头文件包含路径
target_include_directories(${LOG_COMPONENT_NAME} PUBLIC
    # 对外公开的头文件目录
    include 
    src
    ../../ThirdParty/EasyLogger/easylogger/inc
)

# 设置私有头文件包含路径
# target_include_directories(${LOG_COMPONENT_NAME} PRIVATE
#     # 配置文件目录
#     src
    
#     # EasyLogger头文件目录
#     ../../ThirdParty/EasyLogger/easylogger/inc
# )

# 编译定义
# target_compile_definitions(${LOG_COMPONENT_NAME} PUBLIC
#     # 启用EasyLogger输出
#     ELOG_OUTPUT_ENABLE=1
    
#     # 设置输出级别为VERBOSE
#     ELOG_OUTPUT_LVL=5
    
#     # 启用断言检查
#     ELOG_ASSERT_ENABLE=1
    
#     # 行缓冲大小
#     ELOG_LINE_BUF_SIZE=256
    
#     # 换行符定义（STM32使用\r\n）
#     ELOG_NEWLINE_SIGN=\"\\r\\n\"
# )

# 条件编译：如果需要异步输出
option(ELOG_ASYNC_ENABLE "Enable EasyLogger async output" ON)
if(ELOG_ASYNC_ENABLE)
    target_sources(${LOG_COMPONENT_NAME} PRIVATE
        ../../ThirdParty/EasyLogger/easylogger/src/elog_async.c
    )
    target_compile_definitions(${LOG_COMPONENT_NAME} PUBLIC
        ELOG_ASYNC_OUTPUT_ENABLE=1
        ELOG_ASYNC_OUTPUT_BUF_SIZE=2048
    )
endif()

# 条件编译：如果需要文件日志插件
option(ELOG_FILE_ENABLE "Enable EasyLogger file plugin" OFF)
if(ELOG_FILE_ENABLE)
    target_sources(${LOG_COMPONENT_NAME} PRIVATE
        ../../ThirdParty/EasyLogger/easylogger/plugins/file/elog_file.c
        ../../ThirdParty/EasyLogger/easylogger/plugins/file/elog_file_port.c
    )
    target_include_directories(${LOG_COMPONENT_NAME} PUBLIC
        ../../ThirdParty/EasyLogger/easylogger/plugins/file
    )
    target_compile_definitions(${LOG_COMPONENT_NAME} PUBLIC
        ELOG_FILE_ENABLE=1
    )
endif()

# 条件编译：如果需要Flash日志插件
option(ELOG_FLASH_ENABLE "Enable EasyLogger flash plugin" OFF)
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

# 链接STM32 HAL库（如果需要）
target_link_libraries(${LOG_COMPONENT_NAME} PRIVATE
    # 这里可以链接HAL库用于UART输出
    stm32cubemx
)

# 设置编译选项
target_compile_options(${LOG_COMPONENT_NAME} PRIVATE
    -Wall
    -Wextra
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
)

# 导出目标，让其他组件可以使用
set_target_properties(${LOG_COMPONENT_NAME} PROPERTIES
    EXPORT_NAME Log
    OUTPUT_NAME log
)

# 创建别名，方便使用
add_library(Log::Log ALIAS ${LOG_COMPONENT_NAME})

# 打印配置信息
message(STATUS "Log component configuration:")
message(STATUS "  - EasyLogger async: ${ELOG_ASYNC_ENABLE}")
message(STATUS "  - EasyLogger buffer: ${ELOG_BUF_ENABLE}")
message(STATUS "  - EasyLogger file: ${ELOG_FILE_ENABLE}")
message(STATUS "  - EasyLogger flash: ${ELOG_FLASH_ENABLE}")