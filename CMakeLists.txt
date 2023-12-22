cmake_minimum_required(VERSION 3.16)
project(hashiru)

set(CMAKE_CXX_FLAGS -pthread)
set(TARGET ${CMAKE_PROJECT_NAME})
set(CMAKE_CXX_STANDARD 20)

add_library(pugixml STATIC 3rd/pugixml/pugixml.cpp)

set(FILES src/main.cpp)

include_directories("3rd")
include_directories("3rd/pugixml")

add_compile_options(-fno-rtti)

add_executable(${PROJECT_NAME} ${FILES})

target_link_libraries(${PROJECT_NAME} pugixml)



