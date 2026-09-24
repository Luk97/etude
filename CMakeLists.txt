cmake_minimum_required(VERSION 4.4)
project(etude LANGUAGES CXX)

option(ETUDE_ENABLE_ASAN "Build with AddressSanitizer" OFF)
if(ETUDE_ENABLE_ASAN)
    add_compile_options(/fsanitize=address)
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(etude_warnings INTERFACE)
add_library(etude::warnings ALIAS etude_warnings)
if(MSVC)
    target_compile_options(etude_warnings INTERFACE /W4 /WX)
endif()

enable_testing()

add_subdirectory(engine/core)
add_subdirectory(apps/hello)
add_subdirectory(tests)
