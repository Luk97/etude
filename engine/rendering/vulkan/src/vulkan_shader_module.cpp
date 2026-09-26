#include "vulkan_shader_module.h"

#include "vulkan_check.h"

namespace etude::vulkan {

    ShaderModule createShaderModule(VkDevice device, std::span<const std::uint32_t> code) {
        const VkShaderModuleCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size_bytes(),
            .pCode = code.data(),
        };

        VkShaderModule shader = nullptr;
        check(vkCreateShaderModule(device, &info, nullptr, &shader), "vkCreateShaderModule");
        return ShaderModule(shader, {device});
    }
}
