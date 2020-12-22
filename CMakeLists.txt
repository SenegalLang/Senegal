project(senegal C)
cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_STANDARD 99)

file(GLOB src "*/*.h" "*/*.c")

add_executable(senegal ${src} libs/includes/ssocketlib.h libs/ssocketlib.c libs/includes/sfilelib.h libs/sfilelib.c)

if (WIN32)
	set(SENEGAL_EXEC senegal.exe)
	target_link_libraries(senegal ws2_32)
else()
	set(SENEGAL_EXEC senegal)
	target_link_libraries(senegal m)
endif()