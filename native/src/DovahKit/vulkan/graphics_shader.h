#pragma once
#include <optional>
#include <vector>
#include "helpers/eight_cc.h"
#include "_vulkan.h"
#include "_util.h"
#include "pipeline_stage_info.h"
#include "shader_module.h"

namespace vulkanDK {
   class render_pass;
   class surface_renderer;

   class graphics_shader : no_copy {
      public:
         using id_type = cobb::eight_cc;

         enum class area_mode {
            custom,
            whole_surface, // use the whole surface_renderer extent; prevents on_resize from changing the extent
         };
         
         struct bundled_stage_create_info {
            std::vector<VkSpecializationInfo> specializations;
            std::vector<VkPipelineShaderStageCreateInfo> stages;
         };

         struct color_blend {
            bool enabled = true;
            struct {
               VkBlendFactor color = VK_BLEND_FACTOR_ONE;
               VkBlendFactor alpha = VK_BLEND_FACTOR_ONE;
            } source;
            struct {
               VkBlendFactor color = VK_BLEND_FACTOR_ZERO;
               VkBlendFactor alpha = VK_BLEND_FACTOR_ZERO;
            } destination;
            struct {
               VkBlendOp color = VK_BLEND_OP_ADD;
               VkBlendOp alpha = VK_BLEND_OP_ADD;
            } operations;
            VkColorComponentFlags write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
         };

         using resize_handler = void(*)(graphics_shader&, VkExtent2D);

         struct variant_definition {
            std::optional<VkCullModeFlags> face_cull_mode;

            bool operator==(const variant_definition&) const = default;
         };
         struct variant : public variant_definition {
            VkPipeline handle = VK_NULL_HANDLE;

            void setup(graphics_shader& owner, VkViewport, VkRect2D scissor, render_pass&, uint32_t subpass);
            void teardown(graphics_shader& owner);
         };

      public: // constants
         static constexpr color_blend default_no_op_blend = {};
         static constexpr color_blend default_alpha_blend = {
            .enabled = true,
            .source = {
               .color = VK_BLEND_FACTOR_SRC_ALPHA,
               .alpha = VK_BLEND_FACTOR_SRC_ALPHA,
            },
            .destination = {
               .color = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
               .alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            },
            .operations = {
               .color = VK_BLEND_OP_ADD,
               .alpha = VK_BLEND_OP_ADD,
            },
         };

      protected:
         surface_renderer* _owner = nullptr;
         
      public:
         graphics_shader(surface_renderer&);
         graphics_shader(graphics_shader&&);
         ~graphics_shader();

         id_type id;
         resize_handler on_resize = nullptr;
         struct {
            struct {
               area_mode  mode     = area_mode::whole_surface;
               VkRect2D   scissor  = { { 0, 0 }, { 1, 1 } };
               VkViewport viewport = { 0, 0, 1, 1, 0, 1 };
            } area;
            struct {
               struct {
                  bool      enabled = false;
                  VkLogicOp which   = VkLogicOp::VK_LOGIC_OP_COPY;
               } logic_operator;
               std::vector<color_blend> blends; // one per color attachment
            } color_blending;
            struct {
               bool        testing    = true;
               bool        writing    = true;
               VkCompareOp comparison = VK_COMPARE_OP_LESS; // "less" = "closer"
               struct {
                  bool  enabled = false;
                  float min     = 0.0;
                  float max     = 1.0;
               } culling;
            } depth;
            std::vector<VkDescriptorSetLayout> descriptor_set_layouts; // handles; not owned
            std::vector<VkDynamicState> dynamic_states; // VkPipelineDynamicStateCreateInfo
            struct {
               struct {
                  std::vector<VkVertexInputAttributeDescription> attributes;
                  std::vector<VkVertexInputBindingDescription>   bindings;
                  VkPipelineVertexInputStateCreateFlags          flags = 0;
               } vertex;
               VkPipelineInputAssemblyStateCreateInfo triangles;
            } inputs;
            VkPipelineMultisampleStateCreateInfo   multisampling;
            std::vector<VkPushConstantRange>       push_constant_ranges;
            VkPipelineRasterizationStateCreateInfo rasterization; // culling, depth bias, and whether to fill triangles or render wireframes/points
            std::vector<pipeline_stage_info> stages;
            struct {
               bool testing = false;
               VkStencilOpState front = {};
               VkStencilOpState back  = {};
            } stencil;
            struct {
               render_pass* render_pass = nullptr; // not owned; not a smart pointer
               uint32_t     subpass = 0;
            } when;
         } options;
         struct {
            VkPipelineLayout layout = VK_NULL_HANDLE;
            VkPipeline       handle = VK_NULL_HANDLE;
         } pipeline;
         std::vector<variant> variants;

         inline surface_renderer* get_owner() const { return this->_owner; }

         void add_variant(const variant_definition&);
         [[nodiscard]] const variant* get_variant(const variant_definition&) const;
         
         void add_stage(const pipeline_stage_info&);
         void set_layout_info(const std::vector<VkDescriptorSetLayout>&, const std::vector<VkPushConstantRange>& pcr = {});
         void set_render_pass(render_pass*, uint32_t subpass = 0);
         //
         void setup_pipeline_layout();
         void setup_pipeline(VkExtent2D surface); // argument unused if area_mode is not whole_surface
         void teardown_pipeline();

         //
         // Setting up materials' pipeline handles requires knowledge of the final image size 
         // to render to, and so must be re-done every time we rebuild our swap chain in 
         // response to a resize.
         //
         void pre_resize();
         void post_resize(VkExtent2D); // requires extent size and render pass; call after the swap chain and render passes have been handled

         #pragma region options to Vulkan structs
         bundled_stage_create_info stage_create_info() const;

         VkPipelineDepthStencilStateCreateInfo depth_stencil_info() const;

         std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_info() const;
         VkPipelineColorBlendStateCreateInfo color_blend_info(const std::vector<VkPipelineColorBlendAttachmentState>&) const;

         VkPipelineVertexInputStateCreateInfo vertex_info() const;

         VkPipelineDynamicStateCreateInfo dynamic_state_info() const;
         #pragma endregion
   };
}