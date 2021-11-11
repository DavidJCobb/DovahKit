#include "rendered_mesh.h"

namespace vulkanDK {
   rendered_mesh::~rendered_mesh() {
      if (auto*& p = this->anim_state) {
         delete p;
         p = nullptr;
      }
   }

   void rendered_mesh::_on_shader_parameter_change() {
      if (this->pending_delete)
         return;
      this->handled_frames.set_all_out_of_date();
   }
   //
   void rendered_mesh::set_transform(const glm::mat4& in) {
      this->shader_params.transform = in;
      this->_on_shader_parameter_change();
   }

   void rendered_mesh::draw_call(VkCommandBuffer command_buffer) {
      VkDeviceSize offset = 0;
      //
      auto& vib = this->vertex_and_index_buffer;
      //
      if (this->empty()) // object is deleted
         return;
      if (this->pending_delete)
         return;
      //
      vkCmdBindVertexBuffers(command_buffer, 0, 1, &vib.buffer.handle, &offset);
      if constexpr (false) {
         vkCmdBindIndexBuffer(command_buffer, vib.buffer.handle, vib.indices_at, VK_INDEX_TYPE_UINT32);
      } else {
         vkCmdBindIndexBuffer(command_buffer, vib.buffer.handle, vib.indices_at, VK_INDEX_TYPE_UINT16);
      }
      vkCmdDrawIndexed(command_buffer, (uint32_t)vib.index_count, 1, 0, 0, 0);
   }

   void rendered_mesh::mark_for_delete() {
      this->pending_delete = true;
      this->handled_frames.set_all_out_of_date();
   }
   void rendered_mesh::reset() {
      auto& vib = this->vertex_and_index_buffer;
      vib.buffer      = buffer();
      vib.index_count = 0;
      vib.indices_at  = 0;
      //
      if (auto*& p = this->anim_state) {
         delete p;
         p = nullptr;
      }
      this->handled_frames = frame_dirty_state();
   }
}