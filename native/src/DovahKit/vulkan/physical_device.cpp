#include "physical_device.h"
#include "exceptions.h"

namespace vulkanDK {
   physical_device::physical_device(VkPhysicalDevice d) : handle(d) {
      {
         auto&    list  = this->info.queue_families;
         uint32_t count = 0;
         vkGetPhysicalDeviceQueueFamilyProperties(this->handle, &count, nullptr);
         list.resize(count);
         vkGetPhysicalDeviceQueueFamilyProperties(this->handle, &count, list.data());
      }
      //
      auto indexing_features = VkPhysicalDeviceDescriptorIndexingFeaturesEXT{
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
         .pNext = nullptr,
      };
      auto robustness = VkPhysicalDeviceRobustness2FeaturesEXT{
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
         .pNext = &indexing_features,
      };
      auto features = VkPhysicalDeviceFeatures2{
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
         .pNext = &robustness,
      };
      VkPhysicalDeviceProperties properties{};
      vkGetPhysicalDeviceProperties(d, &properties);
      vkGetPhysicalDeviceFeatures2 (d, &features);
      //
      this->info.name = (const char8_t*)properties.deviceName;
      this->info.type = properties.deviceType;
      this->info.vendor_id      = properties.vendorID;
      this->info.device_id      = properties.deviceID;
      this->info.driver_version = properties.driverVersion;
      //
      this->support.vulkan_api_version = properties.apiVersion;
      //
      this->support.independent_blending = features.features.independentBlend;
      this->support.large_points         = features.features.largePoints;
      this->support.max_anisotropic_filtering = 0;
      if (features.features.samplerAnisotropy) {
         this->support.max_anisotropic_filtering = properties.limits.maxSamplerAnisotropy;
      }
      this->support.max_image_dimension_2D       = properties.limits.maxImageDimension2D;
      this->support.max_vertex_index_for_draw    = properties.limits.maxDrawIndexedIndexValue;
      this->support.non_solid_polygon_fill_modes = features.features.fillModeNonSolid;
      //
      auto& sdb = this->support.descriptor_bindings;
      sdb.null_handles   = robustness.nullDescriptor;
      sdb.runtime_array  = indexing_features.runtimeDescriptorArray;
      sdb.variable_count = indexing_features.descriptorBindingVariableDescriptorCount;
      //
      auto& sm = this->support.memory;
      sm.buffer_image_granularity     = properties.limits.bufferImageGranularity;
      sm.max_allocation_count         = properties.limits.maxMemoryAllocationCount;
      sm.max_total_push_constant_size = properties.limits.maxPushConstantsSize;
      //
      auto& sts = this->support.timestamps;
      sts.available       = properties.limits.timestampComputeAndGraphics == VK_TRUE;
      sts.nanosecond_unit = properties.limits.timestampPeriod;
      //
      {
         auto& wl = this->support.wide_lines;
         wl.available         = features.features.wideLines;
         wl.width_granularity = properties.limits.lineWidthGranularity;
         wl.width_range = {
            .minimum = properties.limits.lineWidthRange[0],
            .maximum = properties.limits.lineWidthRange[1],
         };
      }
   }
   physical_device::~physical_device() {
   }

   std::vector<VkExtensionProperties> physical_device::extensions() const {
      using result_t = std::vector<VkExtensionProperties>;

      if (this->handle == VK_NULL_HANDLE)
         return result_t();
      uint32_t count;
      vkEnumerateDeviceExtensionProperties(this->handle, nullptr, &count, nullptr);
      result_t list(count);
      vkEnumerateDeviceExtensionProperties(this->handle, nullptr, &count, list.data());
      return list;

   }
   bool physical_device::has_extension(const char* name) const {
      auto available = this->extensions();
      for (auto& extension : available)
         if (strcmp(extension.extensionName, name) == 0)
            return true;
      return false;
   }
   bool physical_device::has_extensions(const std::vector<const char*>& desired) const {
      auto available = this->extensions();
      bool missing   = false;
      for (auto& name : desired) {
         bool found = false;
         for (auto& extension : available) {
            if (strcmp(extension.extensionName, name) == 0) {
               found = true;
               break;
            }
         }
         if (!found) {
            missing = true;
            break;
         }
      }
      return !missing;
   }

   bool physical_device::has_all_queues(VkQueueFlags flags) const {
      auto& list = this->info.queue_families;
      if (list.empty())
         return flags == 0;
      //
      VkQueueFlags found = 0;
      for (auto& family : list)
         found |= (family.queueFlags & flags);
      return found == flags;
   }

   surface_support_info physical_device::surface_support_details(VkSurfaceKHR surface) const {
      surface_support_info out;
      //
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->handle, surface, &out.capabilities);
      {
         uint32_t count;
         vkGetPhysicalDeviceSurfaceFormatsKHR(this->handle, surface, &count, nullptr);
         if (count != 0) {
            out.formats.resize(count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(this->handle, surface, &count, out.formats.data());
         }
      }
      {
         uint32_t count;
         vkGetPhysicalDeviceSurfacePresentModesKHR(this->handle, surface, &count, nullptr);
         if (count != 0) {
            out.presentation_modes.resize(count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(this->handle, surface, &count, out.presentation_modes.data());
         }
      }
      //
      return out;
   }

   uint32_t physical_device::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
      constexpr uint32_t max_possible_types = std::bit_width(std::numeric_limits<decltype(type_filter)>::max());
      //
      VkPhysicalDeviceMemoryProperties memProperties;
      vkGetPhysicalDeviceMemoryProperties(this->handle, &memProperties);
      //
      auto count = std::min(max_possible_types, memProperties.memoryTypeCount);
      for (uint32_t i = 0; i < count; i++) {
         if ((type_filter & (1 << i)) == 0)
            continue;
         if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
         }
      }
      throw exception("[vulkanDK::physical_device::find_memory_type] Failed to find suitable memory type.");
   }
   VkFormat physical_device::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const {
      for (VkFormat format : candidates) {
         VkFormatProperties props;
         vkGetPhysicalDeviceFormatProperties(this->handle, format, &props);
         //
         if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
         } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
         }
      }
      return VK_FORMAT_UNDEFINED;
   }

   int32_t physical_device::score() const noexcept {
      if (this->handle == VK_NULL_HANDLE)
         return 0;
      int32_t score = 0;
      //
      score += this->support.max_anisotropic_filtering * 10;
      score += (std::min)((uint32_t)8192, this->support.max_image_dimension_2D);
      //
      if (this->info.type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
         score += 10000;
      else if (this->info.type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) // still better than being part of the CPU itself
         score += 200;
      //
      return score;
   }
}