find_package(Vulkan REQUIRED)

add_library(etude_vulkan STATIC)
add_library(etude::vulkan ALIAS etude_vulkan)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)

target_sources(etude_vulkan
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_compile_features(etude_vulkan PUBLIC cxx_std_23)
target_link_libraries(etude_vulkan PUBLIC etude::gfx PRIVATE etude::core etude::warnings Vulkan::Vulkan)
