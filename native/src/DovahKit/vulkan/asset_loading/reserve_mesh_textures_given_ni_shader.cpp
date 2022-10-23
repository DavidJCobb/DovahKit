#include "reserve_mesh_textures_given_ni_shader.h"
#include <QDir>
#include <QString>
#include "helpers/dynamic_fast_cast.h"

#include "../rendered_mesh.h"
#include "../surface_renderer.h"

#include "nif/blocks/BSShaderProperty.h"
#include "nif/blocks/BSShaderTextureSet.h"

#include "nif/blocks/BSLightingShaderProperty.h"
#include "nif/blocks/BSEffectShaderProperty.h"

namespace vulkanDK::asset_loading {
   namespace {
      static size_t _reserve_texture(surface_renderer& sr, QString path) {
         if (!path.endsWith(".dds", Qt::CaseInsensitive))
            return scene::index_of_none;
         path = QDir::cleanPath(path).toLower();
         
         {
            size_t i = sr.scene.reuse_scene_texture(path);
            if (i != scene::index_of_none) {
               if constexpr (false) {
                  qDebug("[vulkanDK::asset_loading::reserve_mesh_textures_given_ni_shader] Reusing texture index %u for texture path <%s>", i, qUtf8Printable(path));
               }
               return i;
            }
         }

         auto texture_index = sr.scene.insert_new_scene_entity<loaded_texture>();
         if (texture_index == scene::index_of_none) {
            qDebug("[vulkanDK::scene_renderer::add_dds_texture] Cannot add new rendered textures. Maximum has been reached.");
            return texture_index;
         }
         if constexpr (false) {
            qDebug("[vulkanDK::scene_renderer::add_dds_texture] Creating new texture at index %u for texture path <%s>", texture_index, qUtf8Printable(path));
         }
         auto& target = sr.scene.entities_of_type<loaded_texture>()[texture_index];
         target.w    = 0;
         target.h    = 0;
         target.path = path;

         target.lifetime.life_state = scene_entities::life_state::active_pending_upload;
         ++sr.uploading.pending_upload_counts.value_for<loaded_texture>();
         // TODO: rethink using `active_pending_upload` without any extra qualifiers. rn the surface renderer 
         //       handles those end-of-frame. background loads aren't in our current plans but if we do decide 
         //       to add that, we'll need some way to tell the surface renderer that this is a background load 
         //       and not an already-loaded texture pending upload.

         return texture_index;
      }
   }

   extern void reserve_mesh_textures_given_ni_shader(surface_renderer& sr, rendered_mesh& mesh, nifDK::block_types::BSShaderProperty* shader) {
      if (!shader) {
         return;
      }
      auto texture_index = loaded_texture_index::none;
      auto normals_index = loaded_texture_index::none;
      if (auto* lighting = cobb::dynamic_fast_cast<nifDK::block_types::BSLightingShaderProperty*>(shader)) {
         if (auto* textures = lighting->texture.paths) {
            const auto& diffuse = textures->textures.diffuse;
            const auto& normals = textures->textures.normal;
            if (!diffuse.empty()) {
               texture_index = _reserve_texture(sr, diffuse.c_str());
            }
            if (!normals.empty()) {
               normals_index = _reserve_texture(sr, normals.c_str());
            }
         }
      } else if (auto* effect = cobb::dynamic_fast_cast<nifDK::block_types::BSEffectShaderProperty*>(shader)) {
         const auto& texture = effect->texture.path;
         if (!texture.empty()) {
            texture_index = _reserve_texture(sr, texture.c_str());
         }
      } else {
         return;
      }
      mesh.texture_indices.diffuse.set(sr, texture_index);
      mesh.texture_indices.normals.set(sr, normals_index);
   }
}