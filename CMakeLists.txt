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

project(XC_LCD C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_library(xc_gui STATIC IMPORTED)
set_target_properties(xc_gui PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/xc_gui/XC_GUI_PICO2W.a")
set_target_properties(xc_gui PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/xc_gui")
# Add executable. Default name is the project name, version 0.1

add_executable(XC_LCD XC_LCD.c )

pico_set_program_name(XC_LCD "XC_LCD")
pico_set_program_version(XC_LCD "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(XC_LCD 0)
pico_enable_stdio_usb(XC_LCD 0)

# Add the standard library to the build
target_link_libraries(XC_LCD 
    pico_stdlib
    hardware_spi
    hardware_gpio )

# Add the standard include files to the build
target_include_directories(XC_LCD PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# add_subdirectory(GUI)

# Add any user requested libraries
target_link_libraries(XC_LCD 
        xc_gui
        )

pico_add_extra_outputs(XC_LCD)

