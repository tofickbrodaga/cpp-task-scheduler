cmake_minimum_required(VERSION 3.14)
project(CppTaskScheduler)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(PkgConfig) 

find_path(JSON_INCLUDE_DIR NAMES nlohmann/json.hpp)
include_directories(${JSON_INCLUDE_DIR})

find_library(SQLITE3_LIB sqlite3)

add_executable(scheduler main.cpp)

target_link_libraries(scheduler PRIVATE Threads::Threads ${SQLITE3_LIB})