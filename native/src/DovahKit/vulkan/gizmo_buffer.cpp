#include "gizmo_buffer.h"
#include "./command_buffer.h"
#include "./surface_renderer.h"

#include "./meshes/gizmo_translate.h"

namespace vulkanDK {
   void gizmo_buffer::setup(surface_renderer& sr) {
      VkDeviceSize size = predefined_meshes::gizmo_translate::vib_size;

      this->vib = sr.create_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      sr.set_debug_object_name(this->vib.handle, "Gizmo VIB");

      {
         buffer staging      = sr.create_staging_buffer(size);
         auto*  staging_data = staging.map_memory();
         {
            using namespace predefined_meshes::gizmo_translate;

            this->indices_start  = vib_i_offset;
            this->vertices_start = 0;

            memcpy(staging_data, mesh.vertices.data(), vib_v_size);
            memcpy(cobb::offset_into(staging_data, vib_i_offset), mesh.indices.data(), vib_i_size);
            //
            this->meshes.translate.first_vertex = 0;
            this->meshes.translate.first_index  = 0;
            this->meshes.translate.index_count  = mesh.indices.size();
            this->meshes.translate.vertex_count = mesh.vertices.size();
         }
         staging.unmap_memory(staging_data);
         this->vib.copy_from(staging);
      }
   }

   void gizmo_buffer::record_draw(surface_renderer& sr, command_buffer& commands) {
      auto command_handle = commands.handle;
      //
      {
         VkDeviceSize offset_i = this->indices_start;
         VkDeviceSize offset_v = this->vertices_start;
         vkCmdBindVertexBuffers(command_handle, 0, 1, &this->vib.handle, &offset_v);
         vkCmdBindIndexBuffer(command_handle, this->vib.handle, offset_i, VK_INDEX_TYPE_UINT16);
      }
      //
      mesh_info info;
      switch (sr.scene.gizmo_state.get_mode()) {
         using enum gizmo_mode;
         case translate: info = this->meshes.translate; break;
         case rotate:    info = this->meshes.rotate;    break;
         case scale:     info = this->meshes.scale;     break;
      }
      //
      vkCmdDrawIndexed(
         command_handle,
         (uint32_t)info.index_count,
         1,
         (uint32_t)info.first_index,
         (uint32_t)info.first_vertex,
         0
      );
   }
}