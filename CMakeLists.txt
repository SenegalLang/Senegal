project(senegal C)
cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_STANDARD 99)

file(GLOB src "*/*.h" "*/*.c")

add_executable(senegal ${src} core/includes/ssocketlib.h core/ssocketlib.c core/includes/sfilelib.h core/sfilelib.c)

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

