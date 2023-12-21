cmake_minimum_required(VERSION 3.20)
project(hashiru)

set(TARGET ${CMAKE_PROJECT_NAME})
set(CMAKE_CXX_STANDARD 20)
set(FILES src/main.cpp)

include_directories("3rd/pugixml/src")
include_directories("3rd")

add_compile_options(-fno-rtti)

add_executable(${PROJECT_NAME} ${FILES})

