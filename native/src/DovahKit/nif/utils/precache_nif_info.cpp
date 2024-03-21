#include "./precache_nif_info.h"
#include "dovah/forms/structs/precached_nif_info.h"
#include "../file.h"

#include "helpers/dynamic_exact_cast.h"
#include "helpers/dynamic_fast_cast.h"

#include "../blocks/BSShaderTextureSet.h"
#include "../blocks/BSValueNode.h"

#include "dovah/bs_hash.h"

namespace nifDK::utils {
   extern dovah::loaded_forms::precached_nif_info precache_nif_info(const file& src) {
      using precached_nif_info = dovah::loaded_forms::precached_nif_info;
      using file_id            = precached_nif_info::file_id;

      precached_nif_info out;

      for (auto* block : src.all_blocks) {
         if (!block) // should never happen
            continue;
         if (auto* casted = cobb::dynamic_exact_cast<block_types::BSShaderTextureSet*>(block)) {
            for (auto& texture : casted->textures.list) {
               if (texture.empty())
                  continue;

               file_id hash(texture);

               auto it = std::find(out.texture_hashes.begin(), out.texture_hashes.end(), hash);
               if (it != out.texture_hashes.end())
                  continue;

               out.texture_hashes.push_back(hash);
            }
            continue;
         }
         if (auto* casted = cobb::dynamic_exact_cast<block_types::BSValueNode*>(block)) {
            auto it = std::find(out.addon_node_ids.begin(), out.addon_node_ids.end(), casted->value);
            if (it != out.addon_node_ids.end())
               continue;

            out.addon_node_ids.push_back(casted->value);
            continue;
         }
      }

      std::sort(out.addon_node_ids.begin(), out.addon_node_ids.end());

      return out;
   }
}