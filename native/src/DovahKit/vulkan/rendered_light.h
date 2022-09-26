#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "helpers/frame_dirty_state.h"
#include "helpers/vertex_index_list.h"
#include "buffer.h"
#include "scene_frame_item.h"

#include "./scene_entities/base.h"

namespace vulkanDK {
   class rendered_light : public scene_entities::base {
      public:
         static constexpr const char* name_single = "light";
         static constexpr const char* name_plural = "lights";
         //
         static constexpr const bool is_drawn = false;
      public:
         rendered_light() {}
         ~rendered_light();

         void mark_for_delete();
         void reset();

         enum class light_type {
            omni,
            omni_shadow,
            hemi_shadow,
            spot_shadow,
         };

         struct frame_drawing_data_type { // pass to the shader via a storage buffer
            alignas(16) glm::mat4  transform;
            alignas(16) glm::mat4  transform_inv;
            alignas(16) glm::vec3  color    = { 0, 0, 0 };
            alignas( 4) float      fade     = 1.0;
            alignas( 4) float      falloff  = 1.0; // falloff exponent, for spotlights only
            alignas( 4) float      fov      = glm::radians(90.0F); // spotlights only
            alignas( 4) float      radius   = 1.0;
            alignas( 4) light_type type     = light_type::omni;
         };
         static_assert(sizeof(frame_drawing_data_type) % 16 == 0, "GLSL's std430 layout requires that this struct be 16-byte-aligned, including within arrays; we need padding to line C++ up.");
         
         frame_drawing_data_type frame_drawing_data;
         
         inline const glm::mat4& transform() const noexcept { return this->frame_drawing_data.transform; }
         void set_transform(const glm::mat4&);

         inline bool can_cast_shadows() const noexcept {
            switch (this->frame_drawing_data.type) {
               using enum light_type;
               case omni_shadow:
               case hemi_shadow:
               case spot_shadow:
                  return true;
            }
            return false;
         }
   };
}