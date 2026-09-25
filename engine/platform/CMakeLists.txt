add_library(etude_platform STATIC)
add_library(etude::platform ALIAS etude_platform)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_platform
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_platform PUBLIC cxx_std_23)
target_compile_definitions(etude_platform PRIVATE UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(etude_platform PUBLIC etude::core PRIVATE etude::warnings user32)
