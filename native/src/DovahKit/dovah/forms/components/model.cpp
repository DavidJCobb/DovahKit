#include "model.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void model::load(tes_subrecord_reader& subrecord) {
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for destruction stages
            subrecord.to_string(this->modelPath);
            break;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for destruction stages
            {
               auto s = subrecord.size();
               this->textureHashes.data.resize(s);
               for (uint32_t i = 0; i < s; i++)
                  subrecord.read(this->textureHashes.data[i]);
            }
            break;
         case 'MODS':
         case 'MO2S':
         case 'DMDS': // for destruction stages
            {
               uint32_t count;
               if (subrecord.read(count)) {
                  for (uint32_t i = 0; i < count; i++) {
                     auto& entry = this->textureSwaps.emplace_back();
                     subrecord.read_length_prefixed_string<4>(entry.nifBlockName);
                     subrecord.read(entry.textureSet);
                     subrecord.read(entry.nifBlockIndex);
                     if (!subrecord.is_in_bounds())
                        break;
                  }
               }
            }
            break;
      }
   }
   /*static*/ void model::generateUseInfo(tes_subrecord_reader& subrecord, form_stub* stub) {
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'MODL':
         case 'MOD2':
         case 'DMDL': // for destruction stages
            break;
         case 'MODT':
         case 'MO2T':
         case 'DMDT': // for destruction stages
            break;
         case 'MODS':
         case 'MO2S':
         case 'DMDS': // for destruction stages
            {
               uint32_t count;
               if (subrecord.read(count)) {
                  for (uint32_t i = 0; i < count; i++) {
                     subrecord.skip_length_prefixed_string<4>();
                     if (subrecord.read(formID))
                        stub->add_outbound_reference(formID);
                     subrecord.skip_bytes(4);
                     if (!subrecord.is_in_bounds())
                        return;
                  }
               }
            }
            break;
      }
   }
}