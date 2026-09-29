add_library(etude_render2d STATIC)
add_library(etude::render2d ALIAS etude_render2d)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_render2d
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_render2d PUBLIC cxx_std_23)
target_link_libraries(etude_render2d
    PUBLIC etude::core etude::math etude::rendering
    PRIVATE etude::warnings)
