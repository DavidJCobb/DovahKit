#pragma once
#include <array>
#include <cstdint>
#include <QFont>
#include <QImage>
#include <QString>
#include <glm/glm.hpp>
#include "helpers/enum_flags.h"
#include "helpers/math.h"
#include "../_vulkan.h"
#include "../buffer.h"
#include "../image.h"

namespace vulkanDK {
   class surface_renderer;
   class swap_chain_image;
}

namespace vulkanDK::overlays {
   //
   // FPS counter overlay which relies on a texture atlas for the label prefix and for the digits. 
   // As such, it will not support kerning, ligatures, etc., between digits, or between the label 
   // and the digits; however, it should be extremely quick to update.
   //
   class fps {
      public:
         // Config:
         static constexpr size_t  max_digits   =  5; // max digits to display
         static constexpr uint8_t display_base = 10; // display numbers in base-10

         // For other compile-time systems' reference:
         static constexpr size_t texture_count = 1;
         static constexpr size_t descriptor_set_index = 1;

         // Magic numbers:
         static constexpr size_t max_visible_value = cobb::pow((size_t)display_base, max_digits) - 1;
         static constexpr size_t vertices_per_quad = 4;
         static constexpr size_t quad_count        = 1 + max_digits;
         static constexpr size_t vertex_count      = quad_count * vertices_per_quad;
         static constexpr size_t index_count       = quad_count * 6;

         using value_type = uint16_t;

      protected:
         struct _vertex {
            glm::vec3 pos;
            glm::vec2 uv;
            //
            static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
            static VkVertexInputBindingDescription getBindingDescription();
         };
         static constexpr size_t _vib_indices_offset = sizeof(_vertex) * vertex_count;

         struct _shader_state {
            float view_w;
            float view_h;
         };

         void _set_quad_x(qreal x, _vertex*, const QRect& glyph);
         void _set_quad_pos(QPoint pos, _vertex*, const QRect& glyph);
         void _set_quad_uv(_vertex*, const QRect& glyph);

         enum class change_flag {
            atlas,
            positions, // digits only
            style,     // indicates the need to regenerate the atlas
            value,
         };
         using change_flags_t = cobb::enum_flags<change_flag, 4>;

         value_type     value        =  0;
         value_type     last_value   = -1;
         change_flags_t change_flags = change_flags_t::with_all_set();
         //
         struct {
            QFont   font;
            QString label;
            float   space_between_digits = 0;
         } style;
         struct {
            QSize size;
            //
            QRect label;
            std::array<QRect, display_base> digits;
            //
            concrete_image image;
         } atlas_info;
         //
         buffer vertex_and_index_buffer;
         struct {
            buffer uniform;
         } shader_params;

      public:
         fps();

         static void create_material_definitions(surface_renderer&);
         void setup_texture_sampler();
         void initialize_descriptor_sets(surface_renderer&, swap_chain_image&);
         void setup_shader_parameter_buffers(surface_renderer&);
         void create_geometry(surface_renderer&);

         void set_font(QFont);
         void set_label(const QString&);
         void set_value(value_type);
         void set_digit_spacing(float);

         bool needs_atlas_update() const;
         bool needs_geometry_update() const;

         QImage generate_atlas();
         void generate_atlas(surface_renderer&, swap_chain_image&);
         void teardown_atlas();

         void update_geometry();
         void update_geometry(void* mapped_vertex_memory);

         // Caller should bind descriptor sets, send necessary push constants, etc., before calling this
         void draw_call(VkCommandBuffer);
   };
}