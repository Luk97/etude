#pragma once

#include "vulkan_image.h"
#include "vulkan_memory.h"

#include <etude/core/image.h>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    /// @brief An image on the GPU that shaders sample, with its memory and its view. The members are destroyed in
    /// reverse order: first the view, then the image, then its memory.
    struct Texture {
        Allocation allocation;
        ImageHandle image;
        ImageView view;
    };

    /// @brief Creates a texture in 8-bit sRGB and copies the pixels of the image into it. Records the copy into the
    /// command buffer and waits until the queue has run it, so the texture is ready when the function returns.
    Texture createTexture(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkQueue queue,
        VkCommandBuffer commands,
        const Image& image
    );
}
