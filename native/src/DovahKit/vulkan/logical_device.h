#pragma once
#include "_vulkan.h"
#include "_util.h"
#include "buffer.h"
#include "physical_device.h"

class DKVulkanInstance;

namespace vulkanDK {
   class surface;
   class surface_renderer;

   class logical_device : no_copy, only_heap_allocate {
      public:
         logical_device(DKVulkanInstance&, const physical_device&, const surface&);
         ~logical_device();

         logical_device(logical_device&&) noexcept;

         DKVulkanInstance& owner;
         const physical_device& physical;
         //
         VkDevice handle = VK_NULL_HANDLE;
         struct {
            VkQueue graphics     = VK_NULL_HANDLE;
            VkQueue presentation = VK_NULL_HANDLE;
         } queues;

         buffer create_buffer(VkDeviceSize size, VkBufferUsageFlags, VkMemoryPropertyFlags);

         void on_dependent_object_created(surface_renderer&);
         void on_dependent_object_deleted(surface_renderer&);

      protected:
         struct {
            //
            // Keep track of dependent objects so that we can enforce top-down destroy (e.g. destroying 
            // a logical device should force teardown on  anything that relied on it, before the device 
            // itself is destroyed).
            //
            std::vector<surface_renderer*> surface_renderers;
         } dependent_objects;
   };
}