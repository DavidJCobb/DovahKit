#include "./magic_effect.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/MagicEffect.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::MagicEffect;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void magic_effect::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      switch (subrecord.signature()) {
         case 'DATA':
            subrecord.skip_bytes(0x0C);
            subrecord.read(this->magic_school);  // 0C
            subrecord.skip_bytes(0x40);          // 10
            subrecord.read(this->casting_type);  // 50
            subrecord.read(this->delivery_type); // 54
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
      {
         auto  src_data = src.magic_skill;
         auto& dst_data = this->magic_school;
         if (src_data != dst_data) {
            dst_data = src_data;
            changed  = true;
         }
      }

      return changed;
   }
}