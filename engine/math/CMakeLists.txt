add_library(etude_math INTERFACE)
add_library(etude::math ALIAS etude_math)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_math
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_math INTERFACE cxx_std_23)
