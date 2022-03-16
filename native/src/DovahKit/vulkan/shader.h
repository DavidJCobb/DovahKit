#pragma once
#include <optional>
#include <vector>
#include "helpers/eight_cc.h"
#include "_vulkan.h"
#include "material.h"

namespace vulkanDK {
   class render_pass;
   class surface_renderer;

   struct shader {
      //
      // Proper usage:
      //  - Define your render passes in advance as render_pass wrappers (even if you can't create the VkRenderPass yet)
      //  - Define your shader objects (these)
      //     - Set the target render pass to the desired wrapper
      //     - Set the area override and layout info as appropriate
      //     - Set up the shader's pipeline layout
      //  - Create the VkRenderPass
      //     - ...generally as part of the process of creating your swap chain
      //  - Set up the shader's pipeline
      //  - When your view is resized, you'll need to reset your swap chain...
      //     - We offer pre_resize and post_resize functions to make this a bit clearer. They'll help you...
      //        - ...tear down the shader's pipeline
      //        - ...update your render passes (i.e. VkRenderPass) as necessary
      //        - ...set up the shader's pipeline again
      //
      public:
         struct area_override_data;
         using  area_override_resize_handler = void(*)(area_override_data&, VkExtent2D);
         struct area_override_data {
            VkViewport viewport;
            VkRect2D   scissor;
            //
            area_override_resize_handler handler = nullptr;
         };

         using id_type = cobb::eight_cc;

         struct variant_definition {
            std::optional<VkCullModeFlags> face_cull_mode;

            bool operator==(const variant_definition&) const = default;
         };
         struct variant : public variant_definition {
            VkPipeline handle = VK_NULL_HANDLE;

            void setup(shader& owner, VkViewport, VkRect2D scissor, render_pass&, uint32_t subpass);
            void teardown(shader& owner);
         };

      public:
         ~shader();

         id_type  id;
         material material;
         material_definition definition;
         struct {
            render_pass* render_pass = nullptr; // not owned; not a smart pointer
            uint32_t     subpass     = 0;
            //
            area_override_data* area_override = nullptr; // owned
            std::vector<VkDescriptorSetLayout> descriptor_set_layouts; // handles; not owned
            std::vector<VkPushConstantRange>   push_constant_ranges;
         } config;
         std::vector<variant> variants;

         surface_renderer* get_owner() const; // requires a material that's been properly set up

         void set_render_pass(render_pass*, uint32_t subpass = 0);
         void set_area_override_info(const area_override_data&);
         void set_layout_info(const std::vector<VkDescriptorSetLayout>&, const std::vector<VkPushConstantRange>& pcr = {});

         void setup_pipeline_layout(surface_renderer&);
         void setup_pipeline(VkExtent2D);

         void add_variant(const variant_definition&);
         [[nodiscard]] const variant* get_variant(const variant_definition&) const;

         //
         // Setting up materials' pipeline handles requires knowledge of the final image size 
         // to render to, and so must be re-done every time we rebuild our swap chain in 
         // response to a resize.
         //
         void pre_resize();
         void post_resize(VkExtent2D); // requires extent size and render pass; call after the swap chain and render passes have been handled
   };
}
