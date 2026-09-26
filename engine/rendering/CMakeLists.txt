add_library(etude_rendering INTERFACE)
add_library(etude::rendering ALIAS etude_rendering)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_rendering
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_rendering INTERFACE cxx_std_23)
target_link_libraries(etude_rendering INTERFACE etude::math)

add_subdirectory(vulkan)
