# RCC Component CMake Configuration

# Define component name
set(RCC_COMPONENT_NAME rcc)

# Add all source files in src directory
file(GLOB RCC_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

# Create a library target for RCC component
add_library(${RCC_COMPONENT_NAME} STATIC ${RCC_SOURCES})

# Add include directories for this component
target_include_directories(${RCC_COMPONENT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# If this component depends on STM32 HAL, you can add it here
target_link_libraries(${RCC_COMPONENT_NAME} PRIVATE stm32cubemx)

# Export the library name for parent CMakeLists.txt
set(RCC_LIBRARY ${RCC_COMPONENT_NAME} PARENT_SCOPE)