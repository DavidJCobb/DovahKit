#include "./movement_type_speeds.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::structs {
   void movement_type_speeds::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      for (size_t i = 0; i < 10; ++i) {
         subrecord.read(this->list[i]);
      }
      if (subrecord.get_containing_record().version() > 27) {
         subrecord.read(this->list[10]);
      }
   }
   void movement_type_speeds::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) const {
      for (size_t i = 0; i < 10; ++i) {
         subrecord.write(this->list[i]);
      }
      if (subrecord.get_containing_record().version() > 27) {
         subrecord.write(this->list[10]);
      }
   }
}