cmake_minimum_required(VERSION 3.4)
project(SysOpy_Project_Ships C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")

link_libraries(ws2_32.lib)

add_executable(server server.c)
add_executable(client client.c)