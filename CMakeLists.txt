cmake_minimum_required(VERSION 3.16)
project(lp2_p1_matrix_multiplication C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(lp2_p1 src/main.c)
target_link_libraries(lp2_p1 PRIVATE Threads::Threads)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(lp2_p1 PRIVATE -O2 -Wall -Wextra)
endif()
