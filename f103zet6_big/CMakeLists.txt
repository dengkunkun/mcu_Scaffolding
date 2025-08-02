cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# 添加测试模式选项
option(BUILD_TESTS "Build with component tests" OFF)

# Set the project name
set(CMAKE_PROJECT_NAME f103zet6_big)

# 如果是测试模式，修改项目名称
if(BUILD_TESTS)
    set(CMAKE_PROJECT_NAME "${CMAKE_PROJECT_NAME}_test")
    message(STATUS "🧪 Test mode enabled - Building ${CMAKE_PROJECT_NAME}")
endif()

set(MCU_FAMILY "STM32F1")
set(MCU_TYPE "STM32F103xE")

add_library(mcu_config INTERFACE)
target_compile_definitions(mcu_config INTERFACE
    ${MCU_TYPE}                    # STM32F103xE
    ${MCU_FAMILY}                  # STM32F1
)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})
add_compile_options(-fdiagnostics-color=always)
# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# 根据构建模式选择不同的app_main
if(BUILD_TESTS)
    add_subdirectory(./app_main_test app_main_build)
    message(STATUS "Using app_main_test for test build")
else()
    add_subdirectory(./app_main app_main_build)
    message(STATUS "Using app_main for normal build")
endif()

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
if(BUILD_TESTS)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx
        app_main_test
        # Add user defined libraries
    )
else()
    target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx
        app_main
        # Add user defined libraries
    )
endif()