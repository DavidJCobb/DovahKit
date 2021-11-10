#pragma once
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "shader_module.h"

namespace vulkanDK {
   class surface_renderer;

   class material_definition {
      public:
         struct color_blend {
            bool enabled = false;
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
         struct stage_info {
            shader_module* module = nullptr; // unowned
            const char*    entry_point_name = nullptr; // function in the shader to call
            VkPipelineShaderStageCreateFlags flags = {};
            VkShaderStageFlagBits            stage = {};
            const VkSpecializationInfo*      specialization_info = nullptr; // can pass parameters to the shader
         };

      public:
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
         struct {
            VkPipelineVertexInputStateCreateInfo   vertex;
            VkPipelineInputAssemblyStateCreateInfo triangles;
         } inputs;
         VkPipelineMultisampleStateCreateInfo   multisampling;
         VkPipelineRasterizationStateCreateInfo rasterization; // culling, depth bias, and whether to fill triangles or render wireframes/points

         void add_stage(const stage_info&);

         std::vector<VkPipelineShaderStageCreateInfo> stage_create_info() const;

         VkPipelineDepthStencilStateCreateInfo depth_stencil_info() const;

         std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_info() const;
         VkPipelineColorBlendStateCreateInfo color_blend_info(const std::vector<VkPipelineColorBlendAttachmentState>&) const;
   };

   class material : no_copy {
      public:
         material(surface_renderer&);
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
         void setup_handle(const material_definition&, VkViewport, VkRect2D scissor, VkRenderPass, uint32_t subpass);

         void teardown_handle();
   };
}