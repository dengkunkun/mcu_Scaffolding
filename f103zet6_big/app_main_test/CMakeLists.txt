cmake_minimum_required(VERSION 3.22)

include(${CMAKE_SOURCE_DIR}/../component/ComponentConfig.cmake)


add_component_subdirectory()

add_third_party_subdirectory()

component_register(
    COMPONENT_NAME app_main_test
    REQUIRES stm32cubemx public worker unity
)

add_version_info_macros(app_main_test)