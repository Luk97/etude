#include "vulkan_pipeline.h"

#include "vulkan_check.h"

#include <cstdint>
#include <span>

namespace etude {

    namespace {

#ifdef __INTELLISENSE__
        // IntelliSense ignores an #include inside an initializer (vscode-cpptools issue 13735), so it gets stand-ins.
        constexpr std::uint32_t vertexShader[] = {0};
        constexpr std::uint32_t fragmentShader[] = {0};
#else
        // glslc turns the shaders into C initializer lists of SPIR-V words during the build.
        constexpr std::uint32_t vertexShader[] =
    #include "vulkan_triangle.vert.inc"
            ;
        constexpr std::uint32_t fragmentShader[] =
    #include "vulkan_triangle.frag.inc"
            ;
#endif

        ShaderModule createShaderModule(VkDevice device, std::span<const std::uint32_t> code) {
            const VkShaderModuleCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size_bytes(),
                .pCode = code.data(),
            };

            VkShaderModule shader = nullptr;
            check(vkCreateShaderModule(device, &info, nullptr, &shader), "vkCreateShaderModule");
            return ShaderModule(shader, ShaderModuleDeleter{device});
        }

        /// @brief Creates an empty layout, because the triangle shaders read neither descriptors nor push constants.
        PipelineLayout createPipelineLayout(VkDevice device) {
            const VkPipelineLayoutCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            };

            VkPipelineLayout layout = nullptr;
            check(vkCreatePipelineLayout(device, &info, nullptr, &layout), "vkCreatePipelineLayout");
            return PipelineLayout(layout, PipelineLayoutDeleter{device});
        }
    }

    VulkanPipeline createTrianglePipeline(VkDevice device, VkFormat colorFormat) {
        // The shader modules are only needed while the pipeline is created.
        const ShaderModule vertex = createShaderModule(device, vertexShader);
        const ShaderModule fragment = createShaderModule(device, fragmentShader);
        const VkPipelineShaderStageCreateInfo stages[] = {
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vertex.get(),
                .pName = "main",
            },
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = fragment.get(),
                .pName = "main",
            },
        };

        // The vertex shader carries the corners itself, so there is no vertex input.
        const VkPipelineVertexInputStateCreateInfo vertexInput{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };
        const VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        };

        // Only the counts, because viewport and scissor are set in every frame as dynamic state.
        const VkPipelineViewportStateCreateInfo viewport{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };
        const VkPipelineRasterizationStateCreateInfo rasterization{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .lineWidth = 1.0f,
        };
        const VkPipelineMultisampleStateCreateInfo multisample{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };
        const VkPipelineColorBlendAttachmentState blendAttachment{
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT,
        };
        const VkPipelineColorBlendStateCreateInfo blend{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &blendAttachment,
        };
        const VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        const VkPipelineDynamicStateCreateInfo dynamic{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamicStates,
        };

        // With dynamic rendering, the pipeline names the formats of its attachments instead of a render pass.
        const VkPipelineRenderingCreateInfo rendering{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colorFormat,
        };

        VulkanPipeline pipeline{
            .layout = createPipelineLayout(device),
        };
        const VkGraphicsPipelineCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &rendering,
            .stageCount = 2,
            .pStages = stages,
            .pVertexInputState = &vertexInput,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewport,
            .pRasterizationState = &rasterization,
            .pMultisampleState = &multisample,
            .pColorBlendState = &blend,
            .pDynamicState = &dynamic,
            .layout = pipeline.layout.get(),
        };

        VkPipeline handle = nullptr;
        check(vkCreateGraphicsPipelines(device, nullptr, 1, &info, nullptr, &handle), "vkCreateGraphicsPipelines");
        pipeline.handle = Pipeline(handle, PipelineDeleter{device});
        return pipeline;
    }
}
