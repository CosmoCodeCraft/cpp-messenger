cmake_minimum_required(VERSION 3.10)
project(Messenger)

set(CMAKE_CXX_STANDARD 17)

add_executable(client Client.cpp)
add_executable(server Server.cpp)
