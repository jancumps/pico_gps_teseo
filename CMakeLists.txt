cmake_minimum_required(VERSION 3.13)

set(PICO_DEOPTIMIZED_DEBUG 1)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
# set(PICO_BOARD seeed_xiao_rp2040)

project(pico_gps_teseo C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()

# libraries and shared resources
# ------------------------------

# sources shared by all examples with uart
add_library(${CMAKE_PROJECT_NAME}_shared_uart
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo/teseo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/reset.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/uart/teseo_communicate.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME}_shared_uart PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/callbackmanager
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/uart
)

target_link_libraries(${CMAKE_PROJECT_NAME}_shared_uart  PUBLIC
        pico_stdlib hardware_gpio hardware_uart
)

# sources shared by all examples with i2c
add_library(${CMAKE_PROJECT_NAME}_shared_i2c
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo/teseo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/reset.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/i2c/teseo_communicate.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME}_shared_i2c PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/teseo
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_teseo_lib/callbackmanager
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/i2c
)

target_link_libraries(${CMAKE_PROJECT_NAME}_shared_i2c 
        pico_stdlib hardware_gpio hardware_i2c
)

# macro to build firmware binary for the examples
macro(__build_firmware example protocol)
message("building firmware for example ${example}, protocol ${protocol}")

add_executable(${CMAKE_PROJECT_NAME}_${example}_${protocol}
        ${CMAKE_CURRENT_SOURCE_DIR}/teseo_${example}.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gps_nmea_lib/nmea/nmea.cpp       
)

target_link_libraries(${CMAKE_PROJECT_NAME}_${example}_${protocol} 
        ${CMAKE_PROJECT_NAME}_shared_${protocol}
)

target_include_directories(${CMAKE_PROJECT_NAME}_${example}_${protocol} PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/gps_nmea_lib/nmea
)

# select the debug output (optional, not used by the GPS interface)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME}_${example}_${protocol} 1)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME}_${example}_${protocol} 0)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME}_${example}_${protocol} )

endmacro()

# build the examples
# ------------------

# build teseo reply response example --------------------------------------

__build_firmware("reply_response" "uart")

__build_firmware("reply_response" "i2c")

# build nmea parse example --------------------------------------

__build_firmware("with_nmea_parse" "uart")

__build_firmware("with_nmea_parse" "i2c")

# build nmea with data processing example --------------------------------------

__build_firmware("nmea_with_data_processing" "uart")

__build_firmware("nmea_with_data_processing" "i2c")
