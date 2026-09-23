add_library(etude_core INTERFACE)
add_library(etude::core ALIAS etude_core)

target_sources(etude_core
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES include/etude/core/vec2.h
)
target_compile_features(etude_core INTERFACE cxx_std_23)
