include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.16.0
    GIT_SHALLOW TRUE
    SYSTEM
)
# Catch2 has to use the standard of the tests, otherwise it leaves out its std::string_view support.
set(CMAKE_CXX_STANDARD 23)
FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)

# The tests of the editor only exist together with the editor.
if(NOT ETUDE_BUILD_EDITOR)
    list(FILTER sources EXCLUDE REGEX "/editor_[^/]*$")
endif()

add_executable(etude_tests ${sources})
target_compile_features(etude_tests PRIVATE cxx_std_23)
target_link_libraries(etude_tests
    PRIVATE etude::core etude::math etude::json etude::platform etude::assets etude::render2d etude::scene
            etude::runtime etude::ui etude::warnings Catch2::Catch2WithMain)
if(ETUDE_BUILD_EDITOR)
    target_link_libraries(etude_tests PRIVATE etude::editor)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(etude_tests)
