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
         std::vector<descriptor_set_layout> descriptor_set_layouts;
         //
         std::vector<render_pass*>   render_passes;  // owns
         std::vector<shader_module*> shader_modules; // owns
         VkSampler texture_sampler = VK_NULL_HANDLE;
         //
         struct {
            uint32_t image_count = 1;
         } configuration;

         std::vector<VkDescriptorSetLayout> descriptor_set_layout_handles() const;

         void teardown();

         void setup_descriptor_set_layouts();

         void setup_command_pool(uint32_t queue_family_index);
         void teardown_command_pool();

         void setup_descriptor_pool();
         void teardown_descriptor_pool();

         void setup_texture_sampler();
         void teardown_texture_sampler();
   };
}
