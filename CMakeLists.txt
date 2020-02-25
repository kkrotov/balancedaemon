cmake_minimum_required(VERSION 3.15)
project(balancedaemon)

set(CMAKE_CXX_STANDARD 14)

add_executable(balancedaemon main.cpp daemonize.cpp sockutil.cpp sockutils.h balanceservice.h)

find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)
target_include_directories(balancedaemon PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(balancedaemon PRIVATE ${PostgreSQL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
