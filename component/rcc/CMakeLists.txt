cmake_minimum_required(VERSION 3.22)

include(${CMAKE_SOURCE_DIR}/../component/ComponentConfig.cmake)

component_register(
    COMPONENT_NAME rcc
    REQUIRES stm32cubemx public
    EXTERN_SRC_DIR ./src
)