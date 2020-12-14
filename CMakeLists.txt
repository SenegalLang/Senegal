project(senegal C)
cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_STANDARD 99)

file(GLOB src "*/*.h" "*/*.c")

add_executable(senegal ${src} src/includes/ssocket.h src/ssocket.c src/includes/sfilelib.h src/sfilelib.c)
target_link_libraries(senegal ws2_32)

if (WIN32)
set(SENEGAL_EXEC senegal.exe)
else()
set(SENEGAL_EXEC senegal)
endif()

add_custom_command(TARGET senegal
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SENEGAL_EXEC} ${CMAKE_SOURCE_DIR})

add_custom_command(TARGET senegal
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SENEGAL_EXEC} ${CMAKE_SOURCE_DIR}/test)

