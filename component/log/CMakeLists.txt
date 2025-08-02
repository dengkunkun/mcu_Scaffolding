cmake_minimum_required(VERSION 3.22)

include(${CMAKE_SOURCE_DIR}/../component/ComponentConfig.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)

component_register(
    COMPONENT_NAME log
    REQUIRES stm32cubemx public
    # EXTERN_SRC 
    EXTERN_SRC_DIR ${CMAKE_SOURCE_DIR}/../ThirdParty/EasyLogger/easylogger/src
    # EXTERN_INCLUDE
)