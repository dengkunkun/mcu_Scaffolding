cmake_minimum_required(VERSION 3.22)

include(${CMAKE_SOURCE_DIR}/../component/ComponentConfig.cmake)


add_component_subdirectory()

add_third_party_subdirectory()

component_register(
    COMPONENT_NAME app_main
    REQUIRES stm32cubemx log uart shell public memory worker
)

add_version_info_macros(app_main)