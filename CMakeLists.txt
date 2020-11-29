cmake_minimum_required(VERSION 3.17)
project(Senegal C)

set(CMAKE_C_STANDARD 99)

file(GLOB src "*/*.h" "*/*.c")

add_executable(Senegal ${src} src/includes/sfiblib.h src/sfiblib.c)
#target_link_libraries(Senegal ws2_32)
