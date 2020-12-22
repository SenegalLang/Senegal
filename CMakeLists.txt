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

add_executable(senegal ${src})

if (WIN32)
	set(SENEGAL_EXEC senegal.exe)
	target_link_libraries(senegal ws2_32)
else()
	set(SENEGAL_EXEC senegal)
	target_link_libraries(senegal m)
endif()

add_custom_command(TARGET senegal
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SENEGAL_EXEC} ${CMAKE_SOURCE_DIR})

add_custom_command(TARGET senegal
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SENEGAL_EXEC} ${CMAKE_SOURCE_DIR}/test)
