#pragma once
#include <type_traits>
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "shader_module.h"

namespace vulkanDK {
   class surface_renderer;

   class material_definition {
      public:
         struct stage_specialization_info {
            std::vector<std::byte> data;
            std::vector<VkSpecializationMapEntry> fields;

            inline bool empty() const { return this->data.empty(); }

            stage_specialization_info() {}
            template<typename T> stage_specialization_info(const T& d, const std::vector<VkSpecializationMapEntry>& f) : fields(f) {
               this->data.resize(sizeof(T));
               memcpy(this->data.data(), &d, sizeof(T));
            };

            template<typename... T> requires (!(std::is_pointer_v<T> || std::is_same_v<T, std::nullptr_t>) && ...)
            stage_specialization_info(T... values) { // assumes constantIDs starting from 0
               this->data.resize((sizeof(T) + ...));
               this->fields.resize(sizeof...(T));
               //
               uint32_t i = 0;
               uint32_t n = 0;
               auto append = [&i, &n, this]<typename T>(T& v) {
                  memcpy(n + this->data.data(), &v, sizeof(T));
                  this->fields[i] = {
                     .constantID = i,
                     .offset     = n,
                     .size       = sizeof(T),
                  };
                  ++i;
                  n += sizeof(T);
               };
               (append(values), ...);
            }
         };

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
            stage_specialization_info specialization_info; // can pass parameters to the shader
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
         void setup_handle(const material_definition&, VkViewport, VkRect2D scissor, VkRenderPass, uint32_t subpass);

         void teardown_handle();
   };
}