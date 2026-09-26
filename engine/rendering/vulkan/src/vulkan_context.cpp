#include "vulkan_context.h"

#include <etude/core/log.h>

namespace etude::vulkan {

    Context createContext(const Window& window) {
        Context context{
            .instance = createInstance(),
        };
        if constexpr (validationEnabled) {
            context.messenger = createMessenger(context.instance.get());
        }
        context.surface = createSurface(context.instance.get(), window);
        context.gpu = chooseGpu(context.instance.get(), context.surface.get());
        context.device = createDevice(context.gpu);
        vkGetDeviceQueue(context.device.get(), context.gpu.queueFamily, 0, &context.queue);

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.gpu.device, &properties);
        logInfo(
            "Vulkan {}.{}.{} ready, validation {}", VK_API_VERSION_MAJOR(properties.apiVersion),
            VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion),
            validationEnabled ? "on" : "off"
        );
        return context;
    }
}
