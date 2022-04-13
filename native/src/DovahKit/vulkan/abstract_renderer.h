#pragma once
#include <vector>
#include <QString>
#include "_vulkan.h"
#include "_util.h"
#include "descriptor_definitions.h"
#include "physical_device.h"

namespace vulkanDK {
   class render_pass;
   class shader_module;

   class abstract_renderer {
      public:
         struct queue {
            VkQueue  handle = VK_NULL_HANDLE;
            uint32_t index  = 0; // family index
            struct {
               VkCommandPool persistent = VK_NULL_HANDLE;
               VkCommandPool transient  = VK_NULL_HANDLE;
            } command_pools;
            //
            queue* alias_of = nullptr; // e.g. "this is a dedicated transfer queue if one exists, or an alias of the graphics queue otherwise"

            void set_alias_of(queue&);

            void setup_handle(VkDevice, uint32_t index);
            void setup_command_pools(VkDevice);
            //
            void teardown_command_pools(VkDevice);
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

         void setup_descriptor_pool(const std::vector<VkDescriptorPoolSize>&, uint32_t total_set_count);
         void teardown_descriptor_pool();

         void setup_texture_sampler();
         void teardown_texture_sampler();

         shader_module* load_shader_module(QString resource_path);
   };
}
