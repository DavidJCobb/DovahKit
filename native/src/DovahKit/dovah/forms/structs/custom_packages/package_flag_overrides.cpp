#include "./package_flag_overrides.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::custom_packages {
   void package_flag_overrides::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      subrecord.read(this->general.set);
      subrecord.read(this->general.clear);
      subrecord.read(this->interrupt.set);
      subrecord.read(this->interrupt.clear);
      if (subrecord.signature() == subrecord_modern) {
         subrecord.read(this->preferred_speed);
         subrecord.skip_bytes(3);
      } else {
         this->preferred_speed = preferred_movement_speed::run;
      }
   }
   void package_flag_overrides::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write(this->general.set);
      subrecord.write(this->general.clear);
      subrecord.write(this->interrupt.set);
      subrecord.write(this->interrupt.clear);
      if (subrecord.signature() == subrecord_modern) {
         subrecord.write(this->preferred_speed);
         subrecord.skip_bytes(3);
      }
   }
}