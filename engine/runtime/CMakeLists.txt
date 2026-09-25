add_library(etude_runtime STATIC)
add_library(etude::runtime ALIAS etude_runtime)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_runtime
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_runtime PUBLIC cxx_std_23)
target_link_libraries(etude_runtime PUBLIC etude::core etude::platform PRIVATE etude::warnings)
