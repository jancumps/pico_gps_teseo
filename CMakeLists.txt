cmake_minimum_required(VERSION 3.13)

set(PICO_DEOPTIMIZED_DEBUG 1)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
# set(PICO_BOARD seeed_xiao_rp2040)

project(pico_gps_teseo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

add_executable(${CMAKE_PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo/teseo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/teseo_communicate.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME} 
        pico_stdlib hardware_gpio hardware_i2c hardware_uart
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo
${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/callbackmanager
${CMAKE_CURRENT_SOURCE_DIR}/port/pico
)

# select the communication protocol to talk to the Teseo GPS
#add_compile_definitions(GPS_OVER_UART)
add_compile_definitions(GPS_OVER_I2C)

# select the debug output (not used by the GPS interface)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 0)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME} )