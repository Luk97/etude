add_executable(bunnymark main.cpp)
target_compile_features(bunnymark PRIVATE cxx_std_23)
target_compile_definitions(bunnymark PRIVATE BUNNYMARK_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/assets")
target_link_libraries(bunnymark PRIVATE etude::assets etude::core etude::runtime etude::ui etude::warnings)
