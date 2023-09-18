#include "worker_thread_for_meshes.h"
#include "../rendered_nif.h"
#include "../surface_renderer.h"

#include "helpers/string/strieq_ascii.h"
#include "helpers/dynamic_fast_cast.h"
#include "nif/blocks/BSEffectShaderProperty.h"
#include "nif/blocks/BSLightingShaderProperty.h"
#include "nif/blocks/BSShaderProperty.h"
#include "nif/blocks/BSTriShape.h"
#include "nif/blocks/NiAlphaProperty.h"
#include "nif/blocks/NiAVObject.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiObjectNET.h"
#include "nif/blocks/NiSwitchNode.h"
#include "nif/blocks/NiTriShape.h"
#include "nif/blocks/NiTriShapeData.h"
#include "nif/types/SkyrimShaderPropertyFlags.h"

namespace vulkanDK::asset_loading {
   namespace {
      void _queue_mesh_for_delete(surface_renderer& sr, rendered_mesh& mesh) {
         assert(!mesh.owning_nif);
         if (mesh.lifetime.life_state == scene_entities::life_state::active_pending_upload) {
            --sr.uploading.pending_upload_counts.value_for<rendered_mesh>();
         }
         mesh.lifetime.life_state = scene_entities::life_state::pending_delete;
         mesh.lifetime.sync_state.set_all_out_of_date();
         ++sr.scene.entities.pending_deletion_counts.value_for<rendered_mesh>();
      }
      void _queue_mesh_for_transfer(surface_renderer& sr, rendered_mesh& mesh) {
         assert(mesh.active());
         mesh.lifetime.life_state = scene_entities::life_state::active_pending_upload;
         mesh.lifetime.sync_state.set_all_out_of_date();
         ++sr.uploading.pending_upload_counts.value_for<rendered_mesh>();
      }

      void _handle_ni_shader_properties(
         rendered_mesh& mesh,
         const nifDK::block_types::NiAlphaProperty* alpha,
         const nifDK::block_types::BSShaderProperty* shader,
         bool& enable_vertex_alpha,
         bool& enable_vertex_color
      ) {
         if (alpha) {
            mesh.push_params.alpha_test_operation  = (int)alpha->testing.mode;
            mesh.push_params.alpha_test_threshold  = (float)alpha->testing.threshold / 255.0F;
            mesh.push_params.enable_alpha_blending = alpha->blending.enabled ? VK_TRUE : VK_FALSE;
            //
            if (!alpha->testing.enabled)
               mesh.push_params.alpha_test_operation = (int)nifDK::block_types::NiAlphaProperty::test_mode::always;
            if (alpha->blending.enabled)
               mesh.mesh_flags |= rendered_mesh::mesh_flag::requires_oit;
         }
         if (shader) {
            //
            // Skyrim shader flags:
            //
            bool has_shader_flags = false;
            std::array<nifDK::SkyrimShaderPropertyFlags, 2> shader_flags;
            if (auto* casted = dynamic_cast<const nifDK::block_types::BSLightingShaderProperty*>(shader)) {
               shader_flags = casted->shader_flags;
               has_shader_flags = true;
               //
               if (casted->material.alpha < 1.0) {
                  mesh.mesh_flags |= rendered_mesh::mesh_flag::requires_oit;
                  //
                  // TODO: pass this alpha value in
                  //
               }
               mesh.frame_drawing_data.specular_color    = { casted->specular.color.r, casted->specular.color.g, casted->specular.color.b };
               mesh.frame_drawing_data.specular_exponent = casted->material.glossiness;
               mesh.frame_drawing_data.specular_strength = casted->specular.strength / 1000.0F; // NIF uses 999 for max brightness?
            } else if (auto* casted = dynamic_cast<const nifDK::block_types::BSEffectShaderProperty*>(shader)) {
               shader_flags = casted->shader_flags;
               has_shader_flags = true;
            }
            if (has_shader_flags) {
               //
               // NOTE: Even with the Vertex Alpha flag enabled, I believe you still need a NiAlphaProperty to 
               // enable alpha blending in order to see vertex alpha values cause any transparency.
               //
               if (shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::vertex_alpha) {
                  enable_vertex_alpha = alpha != nullptr;
               }
               if (shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::decal) {
                  mesh.mesh_flags |= rendered_mesh::mesh_flag::is_decal;
               }
               if (!(shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::cast_shadows)) {
                  mesh.mesh_flags &= ~rendered_mesh::mesh_flag::cast_shadows;
               }
               if (!(shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::receive_shadows)) {
                  mesh.push_params.receive_shadows = VK_FALSE;
               }
               //
               if (shader_flags[1] & nifDK::SkyrimShaderPropertyFlagB::double_sided) {
                  mesh.mesh_flags |= rendered_mesh::mesh_flag::double_sided;
               }
               if (shader_flags[1] & nifDK::SkyrimShaderPropertyFlagB::vertex_colors) {
                  enable_vertex_color = true;
               }
               //
               // For tree meshes, vertex alpha values are co-opted and used for animations:
               //
               if (shader_flags[1] & nifDK::SkyrimShaderPropertyFlagB::use_tree_animation) {
                  enable_vertex_alpha = false;
               }
            }
         }
      }
      void _ni_triangles_to_mesh_triangles(const std::vector<nifDK::Triangle>& list, rendered_mesh& mesh) {
         auto size = list.size();
         mesh.mesh_data.indices = vertex_index_list(size * 3, uint16_t(0));
         //
         if constexpr (sizeof(nifDK::Triangle) == sizeof(uint16_t) * 3) {
            memcpy(mesh.mesh_data.indices.thin_data(), list.data(), size * sizeof(uint16_t) * 3);
         } else {
            //
            // TODO: If this is ever true, it will truncate vertex indices! What should we do about that?
            //
            auto to = mesh.mesh_data.indices.as_thin_range();
            for (size_t i = 0; i < size; ++i) {
               auto& tri = list[i];
               to[(i * 3) + 0] = tri.vertex_indices[0];
               to[(i * 3) + 1] = tri.vertex_indices[1];
               to[(i * 3) + 2] = tri.vertex_indices[2];
            }
         }
      }

      void _load_BSTriShape(surface_renderer& sr, rendered_mesh& mesh, nifDK::block_types::BSTriShape& data, const glm::mat4& parent_transform) {
         auto size = data.vertices.size();
         if (!size || !data.triangles.size()) {
            data.vulkan_state.mesh_handle = {};
            _queue_mesh_for_delete(sr, mesh);
            return;
         }
         if constexpr (false) {
            qDebug("[surface_renderer::add_BSTriShape_mesh] Handling geometry: %s...", data.name.data());
         }
         //
         mesh.frame_drawing_data.transform = parent_transform * data.transform.to_matrix();
         bool enable_vertex_alpha = false;
         bool enable_vertex_color = false;
         _handle_ni_shader_properties(mesh, data.properties.alpha, data.properties.shader, enable_vertex_alpha, enable_vertex_color);
         {  // Vertices
            mesh.mesh_data.vertices.resize(size);
            auto& list = data.vertices;
            const auto& desc = data.vertex_desc;
            for (size_t i = 0; i < size; ++i) {
               auto& src = list[i];
               auto& dst = mesh.mesh_data.vertices[i];
               //
               dst.pos = src.vertex;
               if (enable_vertex_color) {
                  dst.color = { src.color.r, src.color.g, src.color.b, enable_vertex_alpha ? src.color.a : 1.0 };
               } else {
                  dst.color = { 1.0, 1.0, 1.0, 1.0 };
               }
               dst.normal = src.normal;
               dst.tangent = src.tangent;
               dst.bitangent = src.bitangent;
               dst.uv = src.uv;
            }
         }
         if constexpr (false) {
            qDebug("[surface_renderer::add_BSTriShape_mesh] Loaded %u vertices...", size);
         }
         {  // Triangles
            _ni_triangles_to_mesh_triangles(data.triangles, mesh);
            if constexpr (false) {
               qDebug("[surface_renderer::add_BSTriShape_mesh] Loaded %u triangles...", data.triangles.size());
            }
         }
         {  // Bounding sphere
            auto& dst = mesh.mesh_data.bounding_sphere;
            auto& src = data.bounds;
            dst.center = src.center;
            dst.radius_sq = src.radius * src.radius;
            mesh.frame_drawing_data.bounding_sphere_center = src.center;
            mesh.frame_drawing_data.bounding_sphere_radius = src.radius;
            if constexpr (false) {
               qDebug("[surface_renderer::add_BSTriShape_mesh] Loaded NiBound...");
            }
         }
         //
         // Queue for GPU transfer:
         //
         _queue_mesh_for_transfer(sr, mesh);
         if constexpr (false) {
            qDebug("[surface_renderer::add_BSTriShape_mesh] Vulkan setup complete for geometry: %s.", data.name.c_str());
         }
      }
      void _load_NiTriBasedGeom(surface_renderer& sr, rendered_mesh& mesh, nifDK::block_types::NiTriBasedGeom& geom, const glm::mat4& parent_transform) {
         auto* data = dynamic_cast<nifDK::block_types::NiTriShapeData*>(geom.data);
         if (!data) {
            geom.vulkan_state.mesh_handle = {};
            _queue_mesh_for_delete(sr, mesh);
            return;
         }
         auto size = data->vertices.size();
         if (!size || !data->triangles.size()) {
            geom.vulkan_state.mesh_handle = {};
            _queue_mesh_for_delete(sr, mesh);
            return;
         }
         if constexpr (false) {
            qDebug("[surface_renderer::add_NiGeometry_mesh] Handling geometry: %s...", geom.name.data());
         }
         //
         mesh.frame_drawing_data.transform = parent_transform * geom.transform.to_matrix();
         //
         bool enable_vertex_alpha = false;
         bool enable_vertex_color = false;
         _handle_ni_shader_properties(mesh, geom.properties.alpha, geom.properties.shader, enable_vertex_alpha, enable_vertex_color);
         {  // Triangles
            _ni_triangles_to_mesh_triangles(data->triangles, mesh);
            if constexpr (false) {
               qDebug("[surface_renderer::add_NiGeometry_mesh] Loaded %u triangles...", data->triangles.size());
            }
         }
         {  // Vertices
            mesh.mesh_data.vertices.resize(size);
            auto& vl = data->vertices;
            auto& nl = data->normals;
            auto& tl = data->tangents;
            auto& bl = data->bitangents;
            auto& cl = data->vertex_colors;
            auto& ul = data->uv_sets;
            for (size_t i = 0; i < size; ++i) {
               auto& vert = mesh.mesh_data.vertices[i];
               vert.pos = data->vertices[i];
               if (enable_vertex_color && cl.size()) {
                  if (cl.size()) {
                     vert.color = { cl[i].r, cl[i].g, cl[i].b, enable_vertex_alpha ? cl[i].a : 1.0 };
                  } else {
                     //
                     // If the shader enables vertex colors but the mesh data doesn't actually have any, 
                     // then color it all black.
                     //
                     vert.color = { 0.0, 0.0, 0.0, 1.0 };
                  }
               } else {
                  vert.color = { 1.0, 1.0, 1.0, 1.0 };
               }
               if (ul.size()) {
                  auto& uv = ul[0];
                  vert.uv = uv[i];
               } else {
                  vert.uv = { 0, 0 };
               }
               if (nl.size()) {
                  vert.normal = nl[i];
                  if (tl.size()) {
                     assert(bl.size());
                     vert.tangent = tl[i];
                     vert.bitangent = bl[i];
                  } else {
                     vert.tangent = { 1, 0, 0 };
                     vert.bitangent = { 0, 1, 0 };
                  }
               } else {
                  vert.normal = { 0, 0, 1 }; // this won't be a good default...
               }
            }
         }
         if constexpr (false) {
            qDebug("[surface_renderer::add_NiGeometry_mesh] Loaded %u vertices...", size);
         }
         {  // Bounding sphere
            auto& dst = mesh.mesh_data.bounding_sphere;
            auto& src = data->bounds;
            dst.center = src.center;
            dst.radius_sq = src.radius * src.radius;
            mesh.frame_drawing_data.bounding_sphere_center = src.center;
            mesh.frame_drawing_data.bounding_sphere_radius = src.radius;
            if constexpr (false) {
               qDebug("[surface_renderer::add_NiGeometry_mesh] Loaded NiBound...");
            }
         }
         //
         // Vulkan:
         //
         _queue_mesh_for_transfer(sr, mesh);
         if constexpr (false) {
            qDebug("[surface_renderer::add_NiGeometry_mesh] Vulkan setup complete for geometry: %s.", geom.name.c_str());
         }
      }
   }

   void worker_thread_for_meshes::_load_single_nif(const queued_nif_load& item) {
      assert(item.nif);
      auto& nif = *item.nif;
      assert(nif.root_node);

      using namespace nifDK::block_types;
      {
         using namespace nifDK::block_types;

         struct _import_state {
            glm::mat4 transform;
            bool      is_culled = false;
            bool      is_marker = false;
         };
         _import_state state;
         state.transform = item.transform;

         //
         // NOTE: This process is responsible for fully configuring the `rendered_mesh` data for 
         // the NIF, including the transforms for each mesh. The logic for handling transforms 
         // should be kept roughly consistent with `vulkanDK::helpers::nif::set_root_transform`, 
         // though that function will be much simpler since it only needs to handle transforms.
         //
         nif.root_node->walk_tree(
            state,
            [](NiNode* node, _import_state& state) {
               if (node->parent) {
                  //
                  // NOTE: The root node's NIF-side transform is not honored. It gets 
                  // wholly replaced by the REFR's transform.
                  //
                  state.transform = state.transform * node->transform.to_matrix();
               }
               state.is_culled |= ((node->flags & NiAVObject::flag::culled_by_application) != 0);
               if (!state.is_marker) {
                  state.is_marker = cobb::strieq_ascii(node->name, "EditorMarker");
               }
            },
            [this](NiAVObject* object, const _import_state& state) {
               bool switch_node_cull = false;
               if (cobb::dynamic_fast_cast<NiSwitchNode*>(object->parent)) {
                  //
                  // For NiSwitchNodes, cull out all but the current child.
                  //
                  auto* sn = (NiSwitchNode*)object->parent;
                  if (sn->current_child() != object)
                     switch_node_cull = true;
               }

               rendered_mesh* mesh = nullptr;
               if (auto* geom = dynamic_cast<NiTriBasedGeom*>(object)) {
                  mesh = geom->vulkan_state.mesh_handle.entity();
                  _load_NiTriBasedGeom(this->owner, *geom->vulkan_state.mesh_handle, *geom, state.transform);
               } else if (auto* geom = dynamic_cast<BSTriShape*>(object)) {
                  mesh = geom->vulkan_state.mesh_handle.entity();
                  _load_BSTriShape(this->owner, *geom->vulkan_state.mesh_handle, *geom, state.transform);
               }
               //
               if (mesh) {
                  if (state.is_culled || switch_node_cull || (object->flags & NiAVObject::flag::culled_by_application))
                     mesh->mesh_flags |= rendered_mesh::mesh_flag::culled_by_application;
                  //
                  bool is_marker = state.is_marker;
                  if (!is_marker) {
                     is_marker = cobb::strieq_ascii(((NiObjectNET*)object)->name, "EditorMarker");
                  }
                  if (is_marker)
                     mesh->mesh_flags |= rendered_mesh::mesh_flag::is_editor_marker;
               }
            }
         );
         nif.multi_thread_state.flags &= ~rendered_nif::loading_flag::generating_meshes;
      }
   }
   void worker_thread_for_meshes::run() {
      auto& list = this->owner.loading.meshes.batches[this->index];
      for (auto& item : list) {
         if (item.nif->is_cancel_requested()) {
            item.nif->multi_thread_state.flags &= ~rendered_nif::loading_flag::generating_meshes;
            continue;
         }
         this->_load_single_nif(item);
      }
   }
}