# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set( ENV{IDF_PATH} ${CMAKE_CURRENT_LIST_DIR}/esp-idf )
set( ENV{PATH} ENV{PATH}:${CMAKE_CURRENT_LIST_DIR}/xtensa-esp32-elf/bin:${CMAKE_CURRENT_LIST_DIR}/esp-idf/tools:/usr/bin )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(new-esp32-firmware)
