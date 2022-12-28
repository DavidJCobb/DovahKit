#include "vib.h"
#include <tuple>
#include "helpers/tuples/reduce.h"
#include "helpers/align_offset.h"
#include "helpers/offset_into.h"
#include "../command_buffer.h"
#include "../raycast.h"
#include "../surface_renderer.h"
#include "../scene_gizmo_state.h"

#include "./meshes/rotate.h"
#include "./meshes/translate.h"
#include "./meshes/scale.h"
#include "./index_type.h"

namespace vulkanDK::gizmos {
   namespace {
      // order matters, affecting the W-component written into vertices and subsequently 
      // how the vertex shader culls out models we don't currently wish to show
      constexpr const auto& all_meshes = std::tie(
         meshes::translate::mesh,
         meshes::rotate::mesh,
         meshes::scale::mesh
      );

      static const constexpr size_t all_vertices_count = cobb::tuples::reduce<size_t>(all_meshes, [](size_t prev, const auto& mesh) {
         return prev + mesh.vertices.size();
      });
      static const constexpr size_t all_indices_count = cobb::tuples::reduce<size_t>(all_meshes, [](size_t prev, const auto& mesh) {
         return prev + mesh.indices.size();
      });

      static constexpr struct coalesced_mesh_type {
         std::array<vertex, all_vertices_count> vertices = {};
         std::array<index_type, all_indices_count>  indices = {};
      } coalesced_mesh = []() {
         coalesced_mesh_type result = {};

         size_t model_index = 1; // 0 == gizmo_mode::none
         size_t vi = 0;
         size_t ii = 0;
         cobb::tuples::for_each_value(all_meshes, [&vi, &ii, &model_index, &result](const auto& mesh) {
            for (size_t i = 0; i < mesh.vertices.size(); ++i) {
               auto& dst = result.vertices[vi + i];
               dst = mesh.vertices[i];

               uint32_t w = dst.position.w;
               w = w & 0b11; // keep axis
               w |= (model_index << 2);
               dst.position.w = w;
            }
            for (size_t i = 0; i < mesh.indices.size(); ++i) {
               result.indices[ii + i] = mesh.indices[i] + vi;
            }
            //
            vi += mesh.vertices.size();
            ii += mesh.indices.size();
            //
            ++model_index;
         });

         return result;
      }();
   }

   void vib::setup(surface_renderer& sr) {
      constexpr VkDeviceSize vib_i_offset = cobb::align_offset(all_vertices_count * sizeof(vertex), sizeof(index_type));

      this->vertices_start = 0;
      this->indices_start = vib_i_offset;

      constexpr VkDeviceSize size = vib_i_offset + all_indices_count * sizeof(index_type);

      this->data = sr.create_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      sr.set_debug_object_name(this->data.handle, "Gizmo VIB");

      constexpr mesh_info info_translate = []() {
         using namespace meshes::translate;
         return mesh_info{
            .first_index = 0,
            .index_count = mesh.indices.size(),
            .first_vertex = 0,
            .vertex_count = mesh.vertices.size(),
         };
      }();
      constexpr mesh_info info_rotate = [&prev = info_translate]() {
         using namespace meshes::rotate;
         return mesh_info{
            .first_index = prev.first_index + prev.index_count,
            .index_count = mesh.indices.size(),
            .first_vertex = prev.first_vertex + prev.vertex_count,
            .vertex_count = mesh.vertices.size(),
         };
      }();
      constexpr mesh_info info_scale = [&prev = info_rotate]() {
         //using namespace meshes::scale; // TODO
         return mesh_info{
            .first_index = prev.first_index + prev.index_count,
            .index_count = 0, // TODO
            .first_vertex = prev.first_vertex + prev.vertex_count,
            .vertex_count = 0, // TODO
         };
      }();
      //
      this->meshes = {
         .translate = info_translate,
         .rotate = info_rotate,
         .scale = info_scale,
      };

      {
         buffer staging = sr.create_staging_buffer(size);
         auto* staging_data = staging.map_memory();

         memcpy(staging_data, coalesced_mesh.vertices.data(), coalesced_mesh.vertices.size() * sizeof(vertex));
         memcpy(cobb::offset_into(staging_data, vib_i_offset), coalesced_mesh.indices.data(), coalesced_mesh.indices.size() * sizeof(index_type));

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
      vkCmdDrawIndexed(
         command_handle,
         (uint32_t)all_indices_count,
         1,
         (uint32_t)0,
         (uint32_t)0,
         0
      );
   }

   raycast_hit_data vib::do_raycast(const raycast& rc, const scene_gizmo_state& sgs, axis3D& out_which_axis) const {
      switch (sgs.get_mode()) {
         case gizmo_mode::none:
            return {};
         case gizmo_mode::rotate:
            return meshes::rotate::do_raycast(sgs.transform, rc, out_which_axis);
         case gizmo_mode::scale:
            return meshes::scale::do_raycast(sgs.transform, rc, out_which_axis);
         case gizmo_mode::translate:
            return meshes::translate::do_raycast(sgs.transform, rc, out_which_axis);
      }
      return {};
   }
}