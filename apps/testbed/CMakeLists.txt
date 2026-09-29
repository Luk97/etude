add_executable(testbed main.cpp)
target_compile_features(testbed PRIVATE cxx_std_23)
target_compile_definitions(testbed PRIVATE TESTBED_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/assets")
target_link_libraries(testbed PRIVATE etude::core etude::runtime etude::scene etude::warnings)
