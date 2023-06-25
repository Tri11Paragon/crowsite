cmake_minimum_required(VERSION 3.0)
project(crowsite VERSION 0.0.1)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CROW_FEATURES compression)

find_package(Crow)

add_subdirectory(libs/BLT)
include_directories(include/)

file(GLOB_RECURSE source_files src/*.cpp)

add_executable(crowsite ${source_files})

target_link_libraries(crowsite BLT)
target_link_libraries(crowsite Crow::Crow)
target_compile_options(crowsite PRIVATE -Wall -Wextra -Wpedantic)

if (${ENABLE_ADDRSAN} MATCHES ON)
    target_compile_options(crowsite PRIVATE -fsanitize=address)
    target_link_options(crowsite PRIVATE -fsanitize=address)
endif ()

if (${ENABLE_UBSAN} MATCHES ON)
    target_compile_options(crowsite PRIVATE -fsanitize=undefined)
    target_link_options(crowsite PRIVATE -fsanitize=undefined)
endif ()

if (${ENABLE_TSAN} MATCHES ON)
    target_compile_options(crowsite PRIVATE -fsanitize=thread)
    target_link_options(crowsite PRIVATE -fsanitize=thread)
endif ()
