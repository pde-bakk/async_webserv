cmake_minimum_required(VERSION 3.17)
project(webserv)

add_subdirectory(libft)
include_directories(includes srcs getnextline libft)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -pthread")

add_executable(webserv
		srcs/Base64.cpp
		srcs/Cgi.cpp
		srcs/Client.cpp
		srcs/Connection.cpp
		srcs/Enums.cpp
		srcs/Location.cpp
		srcs/main.cpp
		srcs/RequestParser.cpp
		srcs/ResponseHandler.cpp
		srcs/Server.cpp
		srcs/split.cpp
		srcs/utils.cpp
		includes/Base64.hpp
		includes/Client.hpp
		includes/Cgi.hpp
		includes/Colours.hpp
		includes/Connection.hpp
		includes/Enums.hpp
		includes/libftGnl.hpp
		includes/Location.hpp
		includes/RequestParser.hpp
		includes/ResponseHandler.hpp
		includes/Server.hpp
		getnextline/get_next_line.cpp
		getnextline/get_next_line.hpp
		srcs/Worker.cpp
		includes/Worker.hpp
		srcs/ThreadPool.cpp
		includes/ThreadPool.hpp
		srcs/Mutex.cpp
		includes/Mutex.hpp)

target_link_libraries(webserv PUBLIC libft)
