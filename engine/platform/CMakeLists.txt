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
target_link_libraries(etude_platform PUBLIC etude::core etude::math PRIVATE etude::warnings user32)
# Every program that links the platform runs with UTF-8 as its code page, so that narrow strings in paths mean UTF-8.
target_sources(etude_platform INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/utf8.manifest)
