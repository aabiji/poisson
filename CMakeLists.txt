cmake_minimum_required(VERSION 3.30)
project(chanson VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)

add_executable(chanson
    src/main.cpp
)
target_include_directories(chanson PRIVATE "${CMAKE_SOURCE_DIR}/lib")
target_link_libraries(chanson PRIVATE SQLite::SQLite3)
