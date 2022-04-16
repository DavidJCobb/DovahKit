#pragma once
#include <vector>
#include "helpers/eight_cc.h"
#include "_vulkan.h"
#include "pipeline_stage_info.h"

namespace vulkanDK {
   class surface_renderer;

   struct compute_shader {
      //
      // Proper usage:
      //  - Define your shader objects (these)
      //     - Set the target render pass to the desired wrapper
      //     - Set the area override and layout info as appropriate
      //     - Set up the shader's pipeline layout
      //  - Set up the shader's pipeline
      //  - When your view is resized, you'll need to reset your swap chain...
      //     - We offer pre_resize and post_resize functions to make this a bit clearer. They'll help you...
      //        - ...tear down the shader's pipeline
      //        - ...update your render passes (i.e. VkRenderPass) as necessary
      //        - ...set up the shader's pipeline again
      //
      public:
         using id_type = cobb::eight_cc;

      protected:
         surface_renderer* owner = nullptr;
      public:
         compute_shader() {}
         compute_shader(surface_renderer& sr) : owner(&sr) {}
         ~compute_shader();

         id_type id;
         struct {
            std::vector<VkDescriptorSetLayout> descriptor_set_layouts; // handles; not owned
            pipeline_stage_info stage;
         } config;
         struct {
            struct {
               uint32_t x = 0;
               uint32_t y = 0;
               uint32_t z = 0;
            } local_size;
         } metadata;
         struct {
            VkPipelineLayout layout = VK_NULL_HANDLE;
            VkPipeline       handle = VK_NULL_HANDLE;
         } pipeline;

         inline surface_renderer* get_owner() const { return this->owner; }

         void set_layout_info(const std::vector<VkDescriptorSetLayout>&);

         void setup(surface_renderer& owner);
   };
}
