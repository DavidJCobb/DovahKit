#pragma once
#include <string>
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "surface_support_info.h"

namespace vulkanDK {
   class physical_device : only_heap_allocate {
      public:
         physical_device(VkPhysicalDevice);
         ~physical_device();

         const VkPhysicalDevice handle = VK_NULL_HANDLE;
         struct {
            std::u8string name;
            uint32_t    driver_version = 0;
            uint32_t    vendor_id      = 0;
            uint32_t    device_id      = 0;
            //
            VkPhysicalDeviceType type = VK_PHYSICAL_DEVICE_TYPE_OTHER;
            //
            std::vector<VkQueueFamilyProperties> queue_families;
         } info;
         //
         struct {
            uint32_t vulkan_api_version = 0;
            //
            float    max_anisotropic_filtering    = 0;
            uint32_t max_image_dimension_2D       = 0;          // max texture size
            uint32_t max_vertex_index_for_draw    = 0xFFFFFFFF; // indexed-draw calls cannot use vertex indices higher than this
            bool     non_solid_polygon_fill_modes = false;      // are VK_POLYGON_MODE_POINT and VK_POLYGON_MODE_LINE (wireframe) supported?
            struct {
               bool null_handles   = false;
               bool runtime_array  = false;
               bool variable_count = false;
            } descriptor_bindings;
         } support;

         std::vector<VkExtensionProperties> extensions() const;
         bool has_extension(const char*) const;
         bool has_extensions(const std::vector<const char*>&) const;

         bool has_all_queues(VkQueueFlags) const;

         surface_support_info surface_support_details(VkSurfaceKHR) const;

         uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags) const; // throws out_of_range if no matching type is found
         VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling, VkFormatFeatureFlags) const;

         int32_t score() const noexcept; // quick-and-dirty way to rank cards by their feature set. NOTE: does not test support for device extensions or surfaces
   };
}
