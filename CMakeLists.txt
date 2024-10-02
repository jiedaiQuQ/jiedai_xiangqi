cmake_minimum_required(VERSION 3.13)

project(jiedai_xiangqi LANGUAGES CXX)

set(CLOSECONSOLE OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/ege/include)

add_subdirectory(third_party/ege)
add_subdirectory(raw_code)
add_subdirectory(jiedai_xiangqi)