#include "vib.h"
#include <tuple>
#include "helpers/tuples/reduce.h"
#include "helpers/align_offset.h"
#include "helpers/offset_into.h"
#include "../command_buffer.h"
#include "../surface_renderer.h"

#include "./meshes/rotate.h"
#include "./meshes/translate.h"
#include "./index_type.h"

namespace vulkanDK::gizmos {
   namespace {
      template<size_t Size> std::array<index_type, Size>
      constexpr _advance_mesh_indices(const std::array<index_type, Size>& src, size_t advance_by) {
         std::array<index_type, Size> dst = {};
         for (size_t i = 0; i < Size; ++i) {
            dst[i] = src[i] + advance_by;
         }
         return dst;
      }

      constexpr const auto& all_meshes = std::tie(
         meshes::translate::mesh,
         meshes::rotate::mesh//,
         //meshes::scale::mesh
      );
   }

   void vib::setup(surface_renderer& sr) {
      constexpr size_t vertices_count = cobb::tuples::reduce<size_t>(all_meshes, [](size_t prev, const auto& mesh) {
         return prev + mesh.vertices.size();
      });
      constexpr size_t indices_count = cobb::tuples::reduce<size_t>(all_meshes, [](size_t prev, const auto& mesh) {
         return prev + mesh.indices.size();
      });

      constexpr VkDeviceSize vib_i_offset = cobb::align_offset(vertices_count * sizeof(vertex), sizeof(index_type));
      
      this->vertices_start = 0;
      this->indices_start  = vib_i_offset;

      VkDeviceSize size = vib_i_offset + indices_count * sizeof(index_type);

      this->data = sr.create_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      sr.set_debug_object_name(this->data.handle, "Gizmo VIB");

      constexpr mesh_info info_translate = []() {
         using namespace meshes::translate;
         return mesh_info{
            .first_index  = 0,
            .index_count  = mesh.indices.size(),
            .first_vertex = 0,
            .vertex_count = mesh.vertices.size(),
         };
      }();
      constexpr mesh_info info_rotate = [&prev = info_translate]() {
         using namespace meshes::rotate;
         return mesh_info{
            .first_index  = prev.first_index + prev.index_count,
            .index_count  = mesh.indices.size(),
            .first_vertex = prev.first_vertex + prev.vertex_count,
            .vertex_count = mesh.vertices.size(),
         };
      }();
      constexpr mesh_info info_scale = [&prev = info_rotate]() {
         //using namespace meshes::scale; // TODO
         return mesh_info{
            .first_index  = prev.first_index + prev.index_count,
            .index_count  = 0, // TODO
            .first_vertex = prev.first_vertex + prev.vertex_count,
            .vertex_count = 0, // TODO
         };
      }();
      //
      this->meshes = {
         .translate = info_translate,
         .rotate    = info_rotate,
         .scale     = info_scale,
      };

      constexpr auto _offset_v = [vib_i_offset](const mesh_info& info) {
         return sizeof(vertex) * info.first_vertex;
      };
      constexpr auto _offset_i = [vib_i_offset](const mesh_info& info) {
         return vib_i_offset + sizeof(index_type) * info.first_index;
      };
      constexpr auto _size_v = [](const mesh_info& info) {
         return sizeof(vertex) * info.vertex_count;
      };
      constexpr auto _size_i = [](const mesh_info& info) {
         return sizeof(index_type) * info.index_count;
      };

      {
         buffer staging      = sr.create_staging_buffer(size);
         auto*  staging_data = staging.map_memory();

         {
            constexpr const auto& mesh = meshes::translate::mesh;
            memcpy(cobb::offset_into(staging_data, _offset_v(info_translate)), mesh.vertices.data(), _size_v(info_translate));
            memcpy(cobb::offset_into(staging_data, _offset_i(info_translate)), mesh.indices.data(),  _size_i(info_translate));
         }
         {
            //
            // You might think that we need to "shift" the indices for this mesh so that they 
            // continue from the numbers in the previous mesh. Like, as of this writing, the 
            // translate mesh is configured such that it'll have 702 indices, so you might 
            // think that the indices for the rotate mesh need to start from 702 rather than 
            // from 0. However, they don't.
            // 
            // When we use vkCmdDrawIndexed below, we'll specify the first vertex in the mesh. 
            // Indices are numbered relative to that vertex.
            // 
            // Now, if we were to change how the mesh is drawn, then yeah, we'd have to shift 
            // the indices. For example, if we had the draw call use all three models, with 
            // code in the shader discarding whichever model we don't want to show (perhaps 
            // having the W-component specify both the model and axis), then we'd have to then 
            // shift the indices here for that to work. We might take that approach in an 
            // attempt to allow changing the gizmo model without re-recording command buffers.
            // 
            // In order to shift indices, you would do:
            // 
            //    constexpr auto shifted_indices = _advance_mesh_indices(mesh.indices, info_rotate.first_index);
            //

            constexpr const auto& mesh = meshes::rotate::mesh;
            memcpy(cobb::offset_into(staging_data, _offset_v(info_rotate)), mesh.vertices.data(), _size_v(info_rotate));
            memcpy(cobb::offset_into(staging_data, _offset_i(info_rotate)), mesh.indices.data(),  _size_i(info_rotate));
         }
         staging.unmap_memory(staging_data);
         this->data.copy_from(staging);
      }
   }

   void vib::record_draw(surface_renderer& sr, command_buffer& commands) {
      auto command_handle = commands.handle;
      //
      {
         VkDeviceSize offset_i = this->indices_start;
         VkDeviceSize offset_v = this->vertices_start;
         vkCmdBindVertexBuffers(command_handle, 0, 1, &this->data.handle, &offset_v);
         vkCmdBindIndexBuffer(command_handle, this->data.handle, offset_i, VK_INDEX_TYPE_UINT16);
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