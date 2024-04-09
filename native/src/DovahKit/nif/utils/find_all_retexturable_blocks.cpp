#include "./find_all_retexturable_blocks.h"
#include "../file.h"

#include "helpers/dynamic_exact_cast.h"
#include "helpers/dynamic_fast_cast.h"

#include "../blocks/BSLightingShaderProperty.h"
#include "../blocks/BSShaderTextureSet.h"
#include "../blocks/NiGeometry.h"

namespace nifDK::utils {
   extern std::vector<std::pair<size_t, std::string>> find_all_retexturable_blocks(const file& src) {
      std::vector<std::pair<size_t, std::string>> out;

      for (size_t i = 0; i < src.all_blocks.size(); ++i) {
         const auto* block = src.all_blocks[i];
         if (!block) // should never happen
            continue;

         auto* geom = dynamic_cast<const block_types::NiGeometry*>(block);
         if (!geom)
            continue;
         if (!geom->properties.shader)
            continue;

         auto* bslp = dynamic_cast<const block_types::BSLightingShaderProperty*>(geom->properties.shader);
         if (!bslp)
            continue;
         if (bslp->shader_flags[0] & SkyrimShaderPropertyFlagA::remappable_textures) {
            out.push_back({ i, geom->name });
         }
      }

      return out;
   }
}