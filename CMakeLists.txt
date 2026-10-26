# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(ConfigTest C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(ConfigTest ConfigTest.c ConfigParse.c config.c)

pico_set_program_name(ConfigTest "ConfigTest")
pico_set_program_version(ConfigTest "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(ConfigTest 0)
pico_enable_stdio_usb(ConfigTest 1)

# Add the standard library to the build
target_link_libraries(ConfigTest
    pico_stdlib
    hardware_flash
    hardware_sync)

# Reserve three flash sectors at the end of the 4 MB device. The config image
# uses 10 KB; the extra 2 KB is required for sector-aligned erase operations.
pico_override_flash_size(ConfigTest "4 * 1024 * 1024 - 12 * 1024")
target_compile_definitions(ConfigTest PRIVATE
    CONFIG_STORAGE_BYTES=10240
    CONFIG_FLASH_ERASE_BYTES=12288)

# Add the standard include files to the build
target_include_directories(ConfigTest PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(ConfigTest 
        
        )

pico_add_extra_outputs(ConfigTest)

