#include "vulkan_texture_table.h"

#include "vulkan_check.h"

namespace etude::vulkan {

    namespace {

        constexpr std::uint32_t samplerBinding = 0;
        constexpr std::uint32_t texturesBinding = 1;

        /// @brief Creates the layout of the table. The sampler is fixed in the layout, so it needs no descriptor write.
        /// The textures binding may be partially bound: entries without a texture are fine as long as no shader reads
        /// them.
        DescriptorSetLayout createLayout(VkDevice device, VkSampler sampler) {
            const VkDescriptorSetLayoutBinding bindings[] = {
                {
                    .binding = samplerBinding,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = &sampler,
                },
                {
                    .binding = texturesBinding,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .descriptorCount = maxTextures,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                },
            };
            const VkDescriptorBindingFlags flags[] = {0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT};
            const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingsFlags{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .bindingCount = 2,
                .pBindingFlags = flags,
            };
            const VkDescriptorSetLayoutCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = &bindingsFlags,
                .bindingCount = 2,
                .pBindings = bindings,
            };

            VkDescriptorSetLayout layout = nullptr;
            check(vkCreateDescriptorSetLayout(device, &info, nullptr, &layout), "vkCreateDescriptorSetLayout");
            return DescriptorSetLayout(layout, {device});
        }

        /// @brief Creates a pool with room for exactly one table.
        DescriptorPool createPool(VkDevice device) {
            const VkDescriptorPoolSize sizes[] = {
                {
                    .type = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .descriptorCount = 1,
                },
                {
                    .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .descriptorCount = maxTextures,
                },
            };
            const VkDescriptorPoolCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .maxSets = 1,
                .poolSizeCount = 2,
                .pPoolSizes = sizes,
            };

            VkDescriptorPool pool = nullptr;
            check(vkCreateDescriptorPool(device, &info, nullptr, &pool), "vkCreateDescriptorPool");
            return DescriptorPool(pool, {device});
        }
    }

    TextureTable createTextureTable(VkDevice device) {
        TextureTable table{
            .sampler = createNearestSampler(device),
        };
        table.layout = createLayout(device, table.sampler.get());
        table.pool = createPool(device);

        const VkDescriptorSetLayout layout = table.layout.get();
        const VkDescriptorSetAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = table.pool.get(),
            .descriptorSetCount = 1,
            .pSetLayouts = &layout,
        };
        check(vkAllocateDescriptorSets(device, &info, &table.set), "vkAllocateDescriptorSets");
        return table;
    }

    void writeTexture(VkDevice device, const TextureTable& table, std::uint32_t index, VkImageView view) {
        const VkDescriptorImageInfo image{
            .imageView = view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        const VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = table.set,
            .dstBinding = texturesBinding,
            .dstArrayElement = index,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &image,
        };
        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }
}
