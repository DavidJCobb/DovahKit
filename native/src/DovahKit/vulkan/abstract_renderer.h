#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "descriptor_definitions.h"
#include "material.h"
#include "physical_device.h"

namespace vulkanDK {
   class render_pass;
   class shader_module;

   class abstract_renderer {
      public:
         struct queue {
            VkQueue  handle = VK_NULL_HANDLE;
            uint32_t index  = 0; // family index

            void setup(VkDevice, uint32_t);
         };

      public:
         abstract_renderer() {}
         ~abstract_renderer();

         const physical_device* device_info = nullptr;
         VkDevice logical_device = VK_NULL_HANDLE; // subclasses set it up; we tear it down
         //
         struct {
            VkCommandPool persistent = VK_NULL_HANDLE;
            VkCommandPool transient  = VK_NULL_HANDLE;
         } command_pools;
         VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
         //
         std::vector<render_pass*>   render_passes;  // owns
         std::vector<shader_module*> shader_modules; // owns
         VkSampler texture_sampler = VK_NULL_HANDLE;

         void start_teardown(); // command pool, descriptor pool, etc.
            // ...and then subclass should tear down its descriptor set layouts...
         void end_teardown(); // final teardown of the VkDevice

         void setup_command_pool(uint32_t queue_family_index);
         void teardown_command_pool();

         void setup_descriptor_pool(const std::vector<VkDescriptorPoolSize>&, uint32_t total_set_count);
         void teardown_descriptor_pool();

         void setup_texture_sampler();
         void teardown_texture_sampler();
   };
}
