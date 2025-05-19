cmake_minimum_required(VERSION 3.10)
project(HttpServer)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

include_directories(
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/include
)

file (GLOB_RECURSE SOURCES ${SOURCE_DIR}/*.cpp)
file (GLOB_RECURSE EXTERNAL_SOURCES ${EXTERNAL_DIR}/*.c)

set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -pedantic")

add_executable(${PROJECT_NAME} ${SOURCES} ${EXTERNAL_SOURCES})