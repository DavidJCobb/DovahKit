#pragma once
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "helpers/frame_dirty_state.h"
#include "buffer.h"

namespace vulkanDK {
   struct mesh_animation_state {
      bool  playing  = true;
      float duration = 4.0;
      float elapsed  = 0.0;
   };

   class rendered_mesh {
      protected:
         void _on_shader_parameter_change();
      public:
         rendered_mesh() {}
         ~rendered_mesh();

         rendered_mesh(rendered_mesh&&) noexcept = default;
         rendered_mesh& operator=(rendered_mesh&&) noexcept = default;

         struct shader_parameters { // pass to the shader via a storage buffer
            glm::mat4 transform;
         };
         struct push_constant {
            int32_t object_index;
            int32_t texture_index;
         };
         
         struct {
            buffer   buffer;
            uint32_t indices_at  = 0;
            uint32_t index_count = 0;
         } vertex_and_index_buffer;
         shader_parameters shader_params;
         int32_t texture_index = -1;
         //
         frame_dirty_state handled_frames; // for normal objects: frames that have had shader params synchronized. for pending-delete objects: frames that have been unhooked (when all are unhooked, we can delete the VIB)
         bool pending_delete = false; // unhook the object's vertex-and-index buffer from frames' command buffers; delete it when it's fully unhooked
         //
         mesh_animation_state* anim_state = nullptr; // owns
         
         inline bool empty() const noexcept { return this->vertex_and_index_buffer.buffer.empty(); }
         
         inline const glm::mat4& transform() const noexcept { return this->shader_params.transform; }
         void set_transform(const glm::mat4&);

         // Caller should bind descriptor sets, send necessary push constants, etc., before calling this
         void draw_call(VkCommandBuffer);

         void mark_for_delete();
         void reset();
   };
}