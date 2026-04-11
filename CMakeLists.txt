cmake_minimum_required(VERSION 3.30)
project(chanson VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SQLite3 REQUIRED)

add_executable(chanson src/main.c)

target_include_directories(chanson PRIVATE "${CMAKE_SOURCE_DIR}/lib")
target_link_libraries(chanson PRIVATE SQLite::SQLite3 m)
target_precompile_headers(chanson PRIVATE
    "${CMAKE_SOURCE_DIR}/lib/miniaudio.h"
    "${CMAKE_SOURCE_DIR}/lib/stb_image_write.h"
)
