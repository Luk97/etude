include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.16.0
    GIT_SHALLOW TRUE
    SYSTEM
)
# Catch2 has to use the standard of the tets, otherwise it leaves out its std::string_view support.
set(CMAKE_CXX_STANDARD 23)
FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)

add_executable(etude_tests ${sources})
target_compile_features(etude_tests PRIVATE cxx_std_23)
target_link_libraries(etude_tests PRIVATE etude::core etude::math etude::platform etude::warnings Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(etude_tests)
