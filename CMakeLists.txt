cmake_minimum_required(VERSION 3.0)
project(crowsite VERSION 0.0.1)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CROW_FEATURES compression)

cmake_policy(SET CMP0057 NEW)
#find_package(Crow)

set(CROW_BUILD_TESTS OFF)
set(CROW_BUILD_EXAMPLES OFF)
set(CROW_INSTALL OFF)
add_subdirectory(libs/crow)

find_package(CURL)
find_package(OpenSSL)
find_package(SQLite3)

message("SSL ${OPENSSL_INCLUDE_DIR}")
if (NOT SQLite3_FOUND)
    message("Failed to find SQLite3")
endif ()
message("SQLite ${SQLite3_INCLUDE_DIRS} ${SQLite3_LIBRARIES}")

if (NOT CURL_FOUND)
    message("libcurl is required!")
else ()
    message("${CURL_INCLUDE_DIRS} || ${CURL_LIBRARIES}")
endif ()

add_subdirectory(libs/BLT)
include_directories(include/)
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${SQLite3_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

file(GLOB_RECURSE source_files src/*.cpp)

add_executable(crowsite ${source_files})

target_compile_definitions(crowsite PUBLIC CROW_ENABLE_COMPRESSION)

target_link_libraries(crowsite BLT)
target_link_libraries(crowsite Crow::Crow)
target_link_libraries(crowsite ${CURL_LIBRARIES})
target_link_libraries(crowsite OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(crowsite SQLite::SQLite3)
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
