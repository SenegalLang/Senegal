project(senegal C)
cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

file(GLOB src "*/*.h" "*/*.c")

add_library(senegallib SHARED ${src} src/includes/sloadclib.h src/sloadclib.c)
add_executable(senegal ${src} src/includes/sloadclib.h src/sloadclib.c test/test_programs/cimport/test_cimport.c)

if (WIN32)
	set(SENEGAL_EXEC senegal.exe)
	set(SENEGAL_LIB libsenegallib.dll)
	target_link_libraries(senegal ws2_32 senegallib)
elseif (APPLE)
	set(SENEGAL_EXEC senegal)
	set(SENEGAL_LIB libsenegallib.dylib)
	target_link_libraries(senegal m dl senegallib)
else()
	set(SENEGAL_EXEC senegal)
	set(SENEGAL_LIB libsenegallib.so)
	target_link_libraries(senegal m dl senegallib)
endif()

add_custom_command(TARGET senegal
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SENEGAL_EXEC} ${CMAKE_SOURCE_DIR}/test)

add_custom_command(TARGET senegal
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SENEGAL_EXEC} ${CMAKE_SOURCE_DIR}/bin)

add_custom_command(TARGET senegallib
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SENEGAL_LIB} ${CMAKE_SOURCE_DIR}/bin)