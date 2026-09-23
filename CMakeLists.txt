cmake_minimum_required(VERSION 4.4)

project(etude LANGUAGES CXX)

option(ETUDE_ENABLE_ASAN "Build with AddressSanitizer" OFF)
if(ETUDE_ENABLE_ASAN)
    add_compile_options(/fsanitize=address)
endif()

enable_testing()

add_subdirectory(engine/core)
add_subdirectory(apps/hello)
