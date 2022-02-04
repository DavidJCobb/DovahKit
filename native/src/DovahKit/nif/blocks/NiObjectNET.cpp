#include "NiObjectNET.h"
#include "../reader.h"

#include "BSLightingShaderProperty.h"
#include "NiExtraData.h"
#include "NiTimeController.h"

namespace nifDK::block_types {
   void NiObjectNET::parse(file_reader& reader) {
      if (reader.block_type() == BSLightingShaderProperty::type_name) {
         reader.skip(4);
      }
      reader.read_indexed_string(this->name);
      {
         auto v = reader.version();
         if (v <= NiExtraData::max_version_for_linked_list) {
            this->extra.resize(1);
            reader.read_ref(this->extra[0]);
            if (!this->extra[0]) {
               this->extra.clear();
            } else {
               std::vector<NiExtraData*> full;
               for (auto* node = this->extra[0]; node; node = node->next) {
                  full.push_back(node);
               }
               std::swap(this->extra, full);
               //
               // It's tempting to clear each extra-data's  "next" pointer while we do the above, but we 
               // actually can't. See, multiple NiObjectNETs can refer to the same extra data block, and 
               // heck -- multiple extra data blocks can refer to the same extra data block.
               //
            }
         } else {
            uint32_t count;
            reader.read(count);
            this->extra.resize(count);
            for (uint32_t i = 0; i < count; ++i) {
               reader.read_ref(this->extra[i]);
            }
         }
      }
      reader.read_ref(this->controller);
   }
}