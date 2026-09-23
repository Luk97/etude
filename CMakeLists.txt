cmake_minimum_required(VERSION 4.4)
project(etude LANGUAGES CXX)
add_executable(hello main.cpp)
target_compile_features(hello PRIVATE cxx_std_23)
