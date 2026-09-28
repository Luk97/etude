add_library(etude_scene STATIC)
add_library(etude::scene ALIAS etude_scene)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_scene
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_scene PUBLIC cxx_std_23)
target_link_libraries(etude_scene PUBLIC etude::core etude::math etude::json PRIVATE etude::warnings)
