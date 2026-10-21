add_library(etude_editor STATIC)
add_library(etude::editor ALIAS etude_editor)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_editor
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_editor PUBLIC cxx_std_23)
target_link_libraries(etude_editor
    PUBLIC etude::core etude::math etude::rendering etude::render2d etude::scene etude::ui
    PRIVATE etude::warnings)

add_executable(editor app/main.cpp)
target_compile_features(editor PRIVATE cxx_std_23)
target_compile_definitions(editor PRIVATE EDITOR_START_FOLDER="${CMAKE_SOURCE_DIR}/apps/testbed/assets")
target_link_libraries(editor PRIVATE etude::editor etude::runtime etude::warnings)
