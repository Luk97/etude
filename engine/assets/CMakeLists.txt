add_library(etude_assets STATIC)
add_library(etude::assets ALIAS etude_assets)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_assets
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_assets PUBLIC cxx_std_23)
target_link_libraries(etude_assets PUBLIC etude::core PRIVATE etude::warnings)
