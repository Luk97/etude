add_library(etude_gfx INTERFACE)
add_library(etude::gfx ALIAS etude_gfx)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_gfx
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_gfx INTERFACE cxx_std_23)
target_link_libraries(etude_gfx INTERFACE etude::core)
