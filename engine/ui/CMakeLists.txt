add_library(etude_ui STATIC)
add_library(etude::ui ALIAS etude_ui)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_ui
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_ui PUBLIC cxx_std_23)
target_link_libraries(etude_ui
    PUBLIC etude::core etude::math etude::platform etude::rendering etude::render2d
    PRIVATE etude::warnings)
