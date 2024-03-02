cmake_minimum_required(VERSION 3.11)
project(vhash)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" CACHE INTERNAL "")

# options
option(BUILD_TEST "Build Unit Test" OFF)
option(BUILD_BENCH "Build Benchmark" OFF)
option(FFMPEG5 "Build with FFmpeg@5" ON)

# dependencies
## opencv
find_package(OpenCV REQUIRED core imgproc highgui)
include_directories(${OpenCV_INCLUDE_DIRS})

## ffmpeg
find_package(FFMPEG REQUIRED)
include_directories("${FFMPEG_INCLUDE_DIRS}")
if (FFMPEG5)
    add_definitions(-DFFMPEG5)
endif(FFMPEG5)

## spdlog
find_package(spdlog REQUIRED)

## fftw
find_package(FFTW REQUIRED)
include_directories(PkgConfig::FFTW)

## sqlite3
find_package(SQLite3 REQUIRED)

## wavelib
add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/wavelib")
include_directories("${CMAKE_SOURCE_DIR}/third_party/wavelib/header")

## sqlite_orm
include_directories("${CMAKE_SOURCE_DIR}/third_party/sqlite_orm/include")

## cpptqdm
include_directories("${CMAKE_SOURCE_DIR}/third_party/cpptqdm/include")

## CLI11
include_directories("${CMAKE_SOURCE_DIR}/third_party/CLI11/include")

# directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# include
include_directories(${CMAKE_SOURCE_DIR}/include)

# source
file(GLOB APP_SRC ${CMAKE_SOURCE_DIR}/src/app/*.cpp)
file(GLOB CACHE_SRC ${CMAKE_SOURCE_DIR}/src/cache/*.cpp)
file(GLOB HASH_SRC ${CMAKE_SOURCE_DIR}/src/hash/*.cpp)
file(GLOB SCAN_SRC ${CMAKE_SOURCE_DIR}/src/scan/*.cpp)
file(GLOB UTIL_SRC ${CMAKE_SOURCE_DIR}/src/util/*.cpp)
set(ALL_SRC ${APP_SRC} ${CACHE_SRC} ${HASH_SRC} ${SCAN_SRC} ${UTIL_SRC})

# libraries
set(DEP_LIBRARIES ${OpenCV_LIBS} spdlog::spdlog PkgConfig::FFTW ${FFTW_DOUBLE_THREADS_LIB} wavelib SQLite::SQLite3 ${FFMPEG_LIBS})

# cmd
add_executable(
        vhash
        ${CMAKE_SOURCE_DIR}/cmd/vhash.cpp
        ${ALL_SRC}
)
target_link_libraries(
        vhash
        ${DEP_LIBRARIES}
)

install(TARGETS vhash DESTINATION bin)

# test
if (BUILD_TEST)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(
        imagehash_test
        ${CMAKE_SOURCE_DIR}/tests/imagehash_test.cpp
        ${ALL_SRC}
)
target_link_libraries(
        imagehash_test
        ${GTEST_BOTH_LIBRARIES}
        ${DEP_LIBRARIES}
)

add_executable(
        hash_test
        ${CMAKE_SOURCE_DIR}/tests/hash_test.cpp
        ${ALL_SRC}
)
target_link_libraries(
        hash_test
        ${GTEST_BOTH_LIBRARIES}
        ${DEP_LIBRARIES}
)

add_executable(
        cache_test
        ${CMAKE_SOURCE_DIR}/tests/cache_test.cpp
        ${ALL_SRC}
)
target_link_libraries(
        cache_test
        ${GTEST_BOTH_LIBRARIES}
        ${DEP_LIBRARIES}
)

add_executable(
        scan_test
        ${CMAKE_SOURCE_DIR}/tests/scan_test.cpp
        ${ALL_SRC}
)
target_link_libraries(
        scan_test
        ${GTEST_BOTH_LIBRARIES}
        ${DEP_LIBRARIES}
)

add_test(Test imagehash_test hash_test)
enable_testing()
endif(BUILD_TEST)

# benchmark
if(BUILD_BENCH)
find_package(benchmark REQUIRED)

add_executable(
        imagehash_bench
        ${CMAKE_SOURCE_DIR}/tests/imagehash_bench.cpp
        ${ALL_SRC}
)
target_link_libraries(
        imagehash_bench
        benchmark::benchmark
        ${DEP_LIBRARIES}
)

add_executable(
        hash_bench
        ${CMAKE_SOURCE_DIR}/tests/hash_bench.cpp
        ${ALL_SRC}
)
target_link_libraries(
        hash_bench
        benchmark::benchmark
        ${DEP_LIBRARIES}
)

add_executable(
        cache_bench
        ${CMAKE_SOURCE_DIR}/tests/cache_bench.cpp
        ${ALL_SRC}
)
target_link_libraries(
        cache_bench
        benchmark::benchmark
        ${DEP_LIBRARIES}
)
endif(BUILD_BENCH)
