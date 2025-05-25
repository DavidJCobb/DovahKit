#include "./magic_effect.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/MagicEffect.h"
#include "dovah/core.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::MagicEffect;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void magic_effect::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      switch (subrecord.signature()) {
         case 'DATA':
            subrecord.skip_bytes(0x54);
            subrecord.read(this->casting_type);
            subrecord.read(this->delivery_type);
            break;
      }
   }
   bool magic_effect::update(const dovah::loaded_forms::MagicEffect& src) {
      bool changed = false;
      {
         auto  src_data = src.casting_type;
         auto& dst_data = this->casting_type;
         if (src_data != dst_data) {
            dst_data = src_data;
            changed  = true;
         }
      }
      {
         auto  src_data = src.delivery_type;
         auto& dst_data = this->delivery_type;
         if (src_data != dst_data) {
            dst_data = src_data;
            changed  = true;
         }
      }

      return changed;
   }
}