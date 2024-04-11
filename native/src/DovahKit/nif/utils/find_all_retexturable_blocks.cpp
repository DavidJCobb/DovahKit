#include "./find_all_retexturable_blocks.h"
#include "../file.h"

#include "../blocks/BSLightingShaderProperty.h"
#include "../blocks/BSShaderTextureSet.h"
#include "../blocks/NiAVObject.h"
#include "../blocks/NiGeometry.h"
#include "../blocks/NiNode.h"

namespace nifDK::utils {
   extern std::vector<std::pair<size_t, std::string>> find_all_retexturable_blocks(const file& src) {
      std::vector<std::pair<size_t, std::string>> out;

      size_t current_leaf_index = 0;
      [&current_leaf_index, &out](this auto&& recurse, const block_types::NiAVObject* current_block) -> void {
         if (!current_block)
            return;
         if (auto* node = dynamic_cast<const block_types::NiNode*>(current_block)) {
            for(auto* child : node->children)
               recurse(child);
            return;
         }

         if (auto* geom = dynamic_cast<const block_types::NiGeometry*>(current_block)) {
            if (geom->properties.shader) {
               auto* bslp = dynamic_cast<const block_types::BSLightingShaderProperty*>(geom->properties.shader);
               if (bslp) {
                  if (bslp->shader_flags[0] & SkyrimShaderPropertyFlagA::remappable_textures) {
                     out.push_back({ current_leaf_index, geom->name });
                  }
               }
            }
         }

         ++current_leaf_index;
      }(src.root_node);

      return out;
   }
}