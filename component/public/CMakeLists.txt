cmake_minimum_required(VERSION 3.22)

include(${CMAKE_SOURCE_DIR}/../component/ComponentConfig.cmake)

component_register(
    COMPONENT_NAME public
    REQUIRES stm32cubemx
)