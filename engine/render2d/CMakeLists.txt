add_library(etude_render2d INTERFACE)
add_library(etude::render2d ALIAS etude_render2d)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_render2d
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_render2d INTERFACE cxx_std_23)
target_link_libraries(etude_render2d INTERFACE etude::math etude::rendering)
