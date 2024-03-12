cmake_minimum_required(VERSION 3.13)

project(jiedai_xiangqi LANGUAGES CXX)

include_directories(third_party/ege/include)

add_subdirectory(raw_code)
add_subdirectory(third_party/ege)