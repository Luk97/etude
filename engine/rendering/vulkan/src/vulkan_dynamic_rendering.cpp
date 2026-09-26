#include "vulkan_dynamic_rendering.h"

namespace etude::vulkan {

    void beginRendering(VkCommandBuffer commands, VkImageView target, VkRect2D area, Color clearColor) {
        const VkRenderingAttachmentInfo color{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = target,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {
                .color = {
                    .float32 = {clearColor.r, clearColor.g, clearColor.b, clearColor.a},
                },
            },
        };
        const VkRenderingInfo rendering{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = area,
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color,
        };
        vkCmdBeginRendering(commands, &rendering);

        const VkViewport viewport{
            .width = static_cast<float>(area.extent.width),
            .height = static_cast<float>(area.extent.height),
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(commands, 0, 1, &viewport);
        vkCmdSetScissor(commands, 0, 1, &area);
    }

    void endRendering(VkCommandBuffer commands) {
        vkCmdEndRendering(commands);
    }
}
