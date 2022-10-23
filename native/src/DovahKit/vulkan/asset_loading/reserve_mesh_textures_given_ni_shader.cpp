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
         size_t entity_index;
         bool   already_existed;
         sr.lookup_or_reserve_dds_texture(path, entity_index, already_existed);
         return entity_index;
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