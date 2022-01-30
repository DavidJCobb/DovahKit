#pragma once
//#include "_vulkan.h"
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#include "../VulkanMemoryAllocator/vk_mem_alloc.h"

namespace vulkanDK {
   static constexpr bool use_vma_library = true;
}