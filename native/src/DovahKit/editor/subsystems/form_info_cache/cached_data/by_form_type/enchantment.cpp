#include "./enchantment.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/Enchantment.h"

namespace {
   using loaded_form_type = dovah::loaded_forms::Enchantment;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void enchantment::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      switch (subrecord.signature()) {
         case 'DATA':
            subrecord.skip_bytes(
               sizeof(loaded_form_type::cost) +
               sizeof(loaded_form_type::flags) +
               sizeof(loaded_form_type::casting_type) +
               sizeof(loaded_form_type::charge_amount) +
               sizeof(loaded_form_type::delivery_type)
            );
            subrecord.read(this->enchantment_type); // 14
            break;
      }
   }
   bool enchantment::update(const loaded_form_type& src) {
      bool changed = false;
      {
         auto  src_data = src.enchantment_type;
         auto& dst_data = this->enchantment_type;
         if (src_data != dst_data) {
            dst_data = src_data;
            changed  = true;
         }
      }

      return changed;
   }
}