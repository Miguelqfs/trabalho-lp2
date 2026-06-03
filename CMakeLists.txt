cmake_minimum_required(VERSION 3.16)
project(matmul C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(matmul src/main.c)
target_link_libraries(matmul PRIVATE Threads::Threads)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(matmul PRIVATE -O2 -Wall -Wextra)
endif()
