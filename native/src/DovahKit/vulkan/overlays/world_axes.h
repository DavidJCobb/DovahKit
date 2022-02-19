#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <QFont>
#include <QImage>
#include <QString>
#include <glm/glm.hpp>
#include "helpers/enum_flags.h"
#include "helpers/math.h"
#include "../_vulkan.h"
#include "../buffer.h"
#include "../image.h"
#include "../shader.h"
#include "../vertex_metadata.h"

namespace vulkanDK {
   class surface_renderer;
   class swap_chain_image;
}

namespace vulkanDK::overlays {
   //
   // Overlay for displaying the world axes, i.e. which way the camera is looking.
   //
   class world_axes {
      public:
         // Config:
         static constexpr uint32_t viewport_w = 64;
         static constexpr uint32_t viewport_h = 64;
         static constexpr bool     assume_always_redraw = true;
         //
         static constexpr float axis_arrow_length   = 5;
         static constexpr float axis_line_thickness = 2;

         // For other compile-time systems' reference:
         static constexpr size_t texture_count = 1;
         static constexpr shader::id_type shader_id = "WrldAxes";

         // Magic numbers:
         static constexpr size_t axis_count        = 3;
         static constexpr size_t vertices_per_axis = 2;
         static constexpr size_t vertex_count      = axis_count * vertices_per_axis;
         static constexpr size_t index_count       = vertex_count;

      protected:
         struct _vertex {
            glm::vec3 pos;
            glm::vec3 color;
            float     radius = 0.0; // draw a sphere at this vertex?
            //
            static constexpr std::array<VkVertexInputAttributeDescription, 3> get_attribute_descriptions() {
               return vertex_attributes_from_data<
                  vertex_attribute_offset<decltype(pos),    offsetof(_vertex, pos)>,
                  vertex_attribute_offset<decltype(color),  offsetof(_vertex, color)>,
                  vertex_attribute_offset<decltype(radius), offsetof(_vertex, radius)>//,
               >(0);
            }
            static VkVertexInputBindingDescription get_binding_description() {
               return VkVertexInputBindingDescription{
                  .binding   = 0,
                  .stride    = sizeof(_vertex),
                  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // used for non-instanced rendering
               };
            }
         };
         static constexpr size_t _vib_indices_offset = sizeof(_vertex) * vertex_count;

         static void _update_shader_render_area(shader::area_override_data& aod, VkExtent2D extent);

         struct _shader_state {
            glm::mat4 view;
            glm::mat4 proj;
         };
         
         surface_renderer* owner = nullptr;
         bool active = true;
         struct {
            float thickness = 2.0;
         } style;
         struct {
            glm::vec3 last_camera_rotation;
         } state;
         //
         buffer vertex_and_index_buffer;
         struct {
            buffer uniform;
         } shader_params;

      public:
         world_axes();
         void set_owner(surface_renderer&);

         static void setup_shaders(surface_renderer&);
         void initialize_descriptor_sets(swap_chain_image&);
         void setup_shader_parameter_buffers();
         void create_geometry();

         bool needs_redraw() const;

         void commands_pre_pass(VkCommandBuffer); // rendering commands to execute before entering a render pass
         void draw_call(VkCommandBuffer); // caller should bind descriptor sets, send necessary push constants, etc., before calling this
   };
}