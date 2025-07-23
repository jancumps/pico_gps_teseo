cmake_minimum_required(VERSION 3.28)

set(PICO_DEOPTIMIZED_DEBUG 1)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
# set(PICO_BOARD seeed_xiao_rp2040)

project(pico_gps_teseo C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")

pico_sdk_init()

include(FetchContent)

FetchContent_Declare(callbackmanager
  GIT_REPOSITORY "https://github.com/jancumps/callbackmanager.git"
  GIT_TAG "6be5c03"
  SOURCE_SUBDIR =
)
FetchContent_MakeAvailable(callbackmanager)

FetchContent_Declare(gps_teseo_lib
  GIT_REPOSITORY "https://github.com/jancumps/gps_teseo_lib.git"
  GIT_TAG "740b561"
  SOURCE_SUBDIR =
)
FetchContent_MakeAvailable(gps_teseo_lib)

FetchContent_Declare(gps_nmea_lib
  GIT_REPOSITORY "https://github.com/jancumps/gps_nmea_lib.git"
  GIT_TAG "f20ab28"
  SOURCE_SUBDIR =
)
FetchContent_MakeAvailable(gps_nmea_lib)

# libraries and shared resources
# ------------------------------

# sources shared by all examples with uart
add_library(${CMAKE_PROJECT_NAME}_shared_uart)
target_sources(${CMAKE_PROJECT_NAME}_shared_uart
        PUBLIC
        ${gps_teseo_lib_SOURCE_DIR}/teseo/teseo.cpp
        ${gps_nmea_lib_SOURCE_DIR}/nmea/nmea.cpp       
)
target_sources(${CMAKE_PROJECT_NAME}_shared_uart
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${callbackmanager_SOURCE_DIR}/callbackmanager.cpp
        ${gps_teseo_lib_SOURCE_DIR}/teseo/teseo_iface.cpp
        ${gps_nmea_lib_SOURCE_DIR}/nmea/nmea_iface.cpp       
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/reset.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/uart/teseo_communicate.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME}_shared_uart PUBLIC
)

target_link_libraries(${CMAKE_PROJECT_NAME}_shared_uart
        pico_stdlib hardware_gpio hardware_uart
)

# sources shared by all examples with i2c
add_library(${CMAKE_PROJECT_NAME}_shared_i2c)
target_sources(${CMAKE_PROJECT_NAME}_shared_i2c
        PUBLIC
        ${gps_teseo_lib_SOURCE_DIR}/teseo/teseo.cpp
        ${gps_nmea_lib_SOURCE_DIR}/nmea/nmea.cpp       
)
target_sources(${CMAKE_PROJECT_NAME}_shared_i2c
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${callbackmanager_SOURCE_DIR}/callbackmanager.cpp
        ${gps_teseo_lib_SOURCE_DIR}/teseo/teseo_iface.cpp
        ${gps_nmea_lib_SOURCE_DIR}/nmea/nmea_iface.cpp       
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/reset.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/port/pico/i2c/teseo_communicate.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME}_shared_i2c PUBLIC
)

target_link_libraries(${CMAKE_PROJECT_NAME}_shared_i2c
        pico_stdlib hardware_gpio hardware_i2c
)

# macro to build firmware binary for the examples
macro(__build_firmware example protocol)
message("building firmware for example ${example}, protocol ${protocol}")

add_executable(${CMAKE_PROJECT_NAME}_${example}_${protocol}
        ${CMAKE_CURRENT_SOURCE_DIR}/teseo_${example}.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}_${example}_${protocol} PUBLIC 
        ${CMAKE_PROJECT_NAME}_shared_${protocol}
)

target_include_directories(${CMAKE_PROJECT_NAME}_${example}_${protocol} PUBLIC 
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
