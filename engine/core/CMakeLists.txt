add_library(etude_core STATIC)
add_library(etude::core ALIAS etude_core)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_core
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_core PUBLIC cxx_std_23)
target_link_libraries(etude_core PRIVATE etude::warnings)
