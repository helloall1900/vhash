cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(PROJECT_NAME wavelib)
project(${PROJECT_NAME} C)

include(GNUInstallDirs)

# src root path
set(WAVELIB_SRC_ROOT ${PROJECT_SOURCE_DIR} CACHE PATH "Wavelib source root")
# binary output by default
set(COMMON_BIN_PATH ${CMAKE_BINARY_DIR}/Bin)
set(LIBRARY_OUTPUT_PATH ${COMMON_BIN_PATH}/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ${COMMON_BIN_PATH}/${CMAKE_BUILD_TYPE})
# set where to find additional cmake modules if any
set(CMAKE_MODULE_PATH ${WAVELIB_SRC_ROOT}/cmake ${CMAKE_MODULE_PATH})


set(WAVELIB_VERSION "1.0.0" CACHE STRING "Wavelib version" FORCE)
message(">>> Building Wavelib version: ${WAVELIB_VERSION}")
message(">>> EXECUTABLE_OUTPUT_PATH = ${EXECUTABLE_OUTPUT_PATH}")

# cleanup prefix lib for Unix-like OSes
set(CMAKE_SHARED_MODULE_PREFIX)

# make include globaly visible
set(PROJECT_WIDE_INCLUDE ${WAVELIB_SRC_ROOT}/header)
include_directories(${PROJECT_WIDE_INCLUDE})

include_directories(${COMMON_BIN_PATH})

add_subdirectory(src)
