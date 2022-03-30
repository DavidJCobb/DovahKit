#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "helpers/frame_dirty_state.h"
#include "helpers/vertex_index_list.h"
#include "buffer.h"
#include "scene_frame_item.h"

namespace vulkanDK {
   class rendered_light {
      protected:
         void _on_shader_parameter_change();
      public:
         rendered_light() {}
         ~rendered_light();

         enum class light_type {
            omni,
            omni_shadow,
            hemi_shadow,
            spot_shadow,
         };

         struct shader_parameters { // pass to the shader via a storage buffer
            alignas(16) glm::mat4  transform;
            alignas(16) glm::vec3  color    = { 0, 0, 0 };
            alignas( 4) float      radius   = 1.0;
            alignas( 4) float      fade     = 1.0;
            alignas( 4) light_type type     = light_type::omni;
            alignas( 4) uint32_t   pad[2];
         };
         static_assert(sizeof(shader_parameters) % 16 == 0, "GLSL's std430 layout requires that this struct be 16-byte-aligned, including within arrays; we need padding to line C++ up.");
         
         shader_parameters shader_params;
         //
         frame_dirty_state handled_frames; // for normal objects: frames that have had shader params synchronized. for pending-delete objects: frames that have been unhooked (when all are unhooked, we can delete the VIB)
         scene_frame_item_state life_state = scene_frame_item_state::empty;

         inline bool active() const noexcept { return this->life_state == scene_frame_item_state::active; }
         inline bool empty() const noexcept { return this->life_state == scene_frame_item_state::empty; }
         inline bool pending_delete() const noexcept { return this->life_state == scene_frame_item_state::pending_delete; }
         
         inline const glm::mat4& transform() const noexcept { return this->shader_params.transform; }
         void set_transform(const glm::mat4&);

         void mark_for_delete();
         void reset();

         inline bool can_cast_shadows() const noexcept {
            switch (this->shader_params.type) {
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