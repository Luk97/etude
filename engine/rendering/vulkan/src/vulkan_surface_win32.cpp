// The Win32 surface types are only declared when VK_USE_PLATFORM_WIN32_KHR is defined before vulkan.h, which then
// includes windows.h itself.
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "vulkan_check.h"
#include "vulkan_surface.h"

#include <etude/platform/window.h>

#include <vulkan/vulkan.h>

namespace etude::vulkan {

    const char* surfaceExtensionName() {
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    }

    VkSurfaceKHR createSurface(VkInstance instance, const Window& window) {
        const NativeHandles handles = window.nativeHandles();
        const VkWin32SurfaceCreateInfoKHR info{
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = static_cast<HINSTANCE>(handles.instance),
            .hwnd = static_cast<HWND>(handles.window),
        };

        VkSurfaceKHR surface = nullptr;
        check(vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface), "vkCreateWin32SurfaceKHR");
        return surface;
    }
}
