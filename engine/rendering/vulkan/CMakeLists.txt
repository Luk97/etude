find_package(Vulkan REQUIRED COMPONENTS glslc)

add_library(etude_vulkan STATIC)
add_library(etude::vulkan ALIAS etude_vulkan)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)
file(GLOB_RECURSE shaders CONFIGURE_DEPENDS src/*.vert src/*.frag)

# glslc compiles each shader into a C initializer list of SPIR-V words, which the sources include.
foreach(shader IN LISTS shaders)
    cmake_path(GET shader FILENAME name)
    set(spirv ${CMAKE_CURRENT_BINARY_DIR}/${name}.inc)
    add_custom_command(
        OUTPUT ${spirv}
        COMMAND Vulkan::glslc --target-env=vulkan1.3 -Werror -mfmt=c -o ${spirv} ${shader}
        DEPENDS ${shader}
        COMMENT "Compiling shader ${name}"
        VERBATIM
    )
    list(APPEND sources ${spirv})
endforeach()

target_sources(etude_vulkan
    PRIVATE
        ${sources}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${headers}
)
target_include_directories(etude_vulkan PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_features(etude_vulkan PUBLIC cxx_std_23)
target_link_libraries(etude_vulkan PUBLIC etude::rendering PRIVATE etude::core etude::platform etude::warnings Vulkan::Vulkan)
