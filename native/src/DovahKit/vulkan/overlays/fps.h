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
#include "../graphics_shader.h"
#include "../image.h"

namespace vulkanDK {
   class frame_in_flight;
   class surface_renderer;
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
         static constexpr size_t  history_size = 10;
         static constexpr bool    show_history_average      = true;
         static constexpr bool    assume_always_redraw      = true; // generally sensible; the FPS will likely change every frame
         static constexpr bool    persistent_staging_buffer = true; // useful when (assume_always_redraw) is (true)

         // For other compile-time systems' reference:
         static constexpr size_t texture_count = 1;
         static constexpr graphics_shader::id_type shader_id = "FPSCount";

         // Magic numbers:
         static constexpr size_t max_visible_value = cobb::pow((size_t)display_base, max_digits) - 1;
         static constexpr size_t vertices_per_quad = 4;
         static constexpr size_t quad_count        = 1 + max_digits;
         static constexpr size_t vertex_count      = quad_count * vertices_per_quad;
         static constexpr size_t index_count       = quad_count * 6;

         using value_type = int32_t;
         static constexpr value_type max_value = std::numeric_limits<value_type>::max();

      protected:
         struct _vertex {
            glm::vec3 pos;
            glm::vec2 uv;
            //
            static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
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

         value_type value = 0;
         struct {
            double average = 0.0;
            size_t count   = 0;
         } history;
         change_flags_t change_flags = change_flags_t::with_all_set();
         //
         struct {
            QFont   font;
            QString label;
            bool    omit_leading_zeroes  = true;
            float   space_between_digits = 0; // in pixels
         } style;
         struct {
            QSize size;
            //
            QRect label;
            std::array<QRect, display_base> digits;
            //
            owned_image_and_view image;
         } atlas_info;
         //
         buffer vertex_and_index_buffer;
         buffer vi_staging_buffer;
         struct {
            buffer uniform;
         } shader_params;

      public:
         fps();

         static void setup_shaders(surface_renderer&);
         void initialize_descriptor_sets(surface_renderer&, frame_in_flight&);
         void setup_shader_parameter_buffers(surface_renderer&);
         void create_geometry(surface_renderer&);

         void set_font(QFont);
         void set_label(const QString&);
         void set_value(value_type);
         void set_digit_spacing(float);

         bool needs_atlas_update() const;
         bool needs_geometry_update() const;

         void handle_resize(surface_renderer&, frame_in_flight&);

         QImage generate_atlas();
         void generate_atlas(surface_renderer&, frame_in_flight&);
         void teardown_atlas();

         void update_geometry(surface_renderer&);
         void update_geometry(void* mapped_vertex_memory);

         void commands_pre_pass(VkCommandBuffer); // rendering commands to execute before entering a render pass
         void draw_call(VkCommandBuffer); // caller should bind descriptor sets, send necessary push constants, etc., before calling this
   };
}