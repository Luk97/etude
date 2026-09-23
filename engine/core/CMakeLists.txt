add_library(etude_core)
add_library(etude::core ALIAS etude_core)

target_sources(etude_core
    PUBLIC
        FILE_SET CXX_MODULES
        FILES core.cppm
)
target_compile_features(etude_core PUBLIC cxx_std_23)
