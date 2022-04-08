#pragma once
#include <type_traits>
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "shader_module.h"
#include "pipeline_stage_info.h"

namespace vulkanDK {
   class render_pass;
   class surface_renderer;

   class material_definition {
      public:
         using stage_specialization_info = pipeline_stage_specialization_info;
         using stage_info = pipeline_stage_info;

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

      public:
         material_definition();

         std::vector<stage_info> stages;
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
         struct {
            bool testing = false;
            VkStencilOpState front = {};
            VkStencilOpState back  = {};
         } stencil;
         //
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
         VkPipelineRasterizationStateCreateInfo rasterization; // culling, depth bias, and whether to fill triangles or render wireframes/points

         void add_stage(const stage_info&);

         struct bundled_stage_create_info {
            std::vector<VkSpecializationInfo> specializations;
            std::vector<VkPipelineShaderStageCreateInfo> stages;
         };
         bundled_stage_create_info stage_create_info() const;

         VkPipelineDepthStencilStateCreateInfo depth_stencil_info() const;

         std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_info() const;
         VkPipelineColorBlendStateCreateInfo color_blend_info(const std::vector<VkPipelineColorBlendAttachmentState>&) const;

         VkPipelineVertexInputStateCreateInfo vertex_info() const;

         VkPipelineDynamicStateCreateInfo dynamic_state_info() const;
   };

   class material : no_copy {
      public:
         material() {};
         ~material();

         material(material&&) noexcept;
         material& operator=(material&&) noexcept;

         surface_renderer* owner = nullptr;

         struct {
            VkRect2D   scissor;  // what part of the framebuffer to retain (like a write-mask)
            VkViewport viewport; // what part of the framebuffer to render to
         } config;
         struct {
            VkPipelineLayout layout = VK_NULL_HANDLE;
            VkPipeline       handle = VK_NULL_HANDLE;
         } pipeline;

         void setup_layout(const std::vector<VkDescriptorSetLayout>&, const std::vector<VkPushConstantRange>& pcr = {});
         void setup_handle(const material_definition&, VkViewport, VkRect2D scissor, render_pass&, uint32_t subpass);

         void teardown_handle();
   };
}