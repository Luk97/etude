add_library(etude_json STATIC)
add_library(etude::json ALIAS etude_json)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_json
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_json PUBLIC cxx_std_23)
target_link_libraries(etude_json PRIVATE etude::warnings)
