add_library(etude_core INTERFACE)
add_library(etude::core ALIAS etude_core)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_core
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_core INTERFACE cxx_std_23)
