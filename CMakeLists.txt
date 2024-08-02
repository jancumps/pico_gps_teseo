cmake_minimum_required(VERSION 3.13)

set(PICO_DEOPTIMIZED_DEBUG 1)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
# set(PICO_BOARD seeed_xiao_rp2040)

project(pico_gps_teseo C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()

# teseo reply response example --------------------------------------

add_executable(${CMAKE_PROJECT_NAME}_reply_response
        ${CMAKE_CURRENT_SOURCE_DIR}/teseo_reply_response.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo/teseo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/teseo_communicate.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}_reply_response 
        pico_stdlib hardware_gpio hardware_i2c hardware_uart
)

target_include_directories(${CMAKE_PROJECT_NAME}_reply_response PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo
${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/callbackmanager
${CMAKE_CURRENT_SOURCE_DIR}/port/pico
)

# select the communication protocol to talk to the Teseo GPS
#add_compile_definitions(GPS_OVER_UART)
add_compile_definitions(GPS_OVER_I2C)

# select the debug output (not used by the GPS interface)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME}_reply_response 1)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME}_reply_response 0)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME}_reply_response )

# nmea parse example --------------------------------------

add_executable(${CMAKE_PROJECT_NAME}_nmea_parse
        ${CMAKE_CURRENT_SOURCE_DIR}/teseo_with_nmea_parse.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo/teseo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_nmea_lib/nmea/nmea.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/teseo_communicate.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}_nmea_parse 
        pico_stdlib hardware_gpio hardware_i2c hardware_uart
)

target_include_directories(${CMAKE_PROJECT_NAME}_nmea_parse PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo
${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/callbackmanager
${CMAKE_CURRENT_SOURCE_DIR}/gps_nmea_lib/nmea
${CMAKE_CURRENT_SOURCE_DIR}/port/pico
)

# select the communication protocol to talk to the Teseo GPS
#add_compile_definitions(GPS_OVER_UART)
add_compile_definitions(GPS_OVER_I2C)

# select the debug output (not used by the GPS interface)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME}_nmea_parse 1)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME}_nmea_parse 0)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME}_nmea_parse )