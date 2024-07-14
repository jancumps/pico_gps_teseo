cmake_minimum_required(VERSION 3.13)

set(PICO_DEOPTIMIZED_DEBUG 1)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
# set(PICO_BOARD seeed_xiao_rp2040)

project(pico_gps_teseo_i2c C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

add_executable(${CMAKE_PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/teseo/teseo.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME} 
        pico_stdlib hardware_gpio hardware_i2c
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/teseo
${CMAKE_CURRENT_SOURCE_DIR}/callbackmanager
)

pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 0)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME} )