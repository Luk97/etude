add_executable(hello main.cpp)
target_compile_features(hello PRIVATE cxx_std_23)
target_link_libraries(hello PRIVATE etude::core)
