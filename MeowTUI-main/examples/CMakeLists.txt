cmake_minimum_required(VERSION 3.16)
project(MeowTUI_Examples LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC ../src/*.cpp)

add_library(m STATIC ${SRC})
target_include_directories(m PUBLIC ../include)

add_executable(minimal minimal.cpp)
add_executable(demo demo.cpp)
add_executable(filemanager filemanager.cpp)

target_link_libraries(minimal m)
target_link_libraries(demo m)
target_link_libraries(filemanager m)
