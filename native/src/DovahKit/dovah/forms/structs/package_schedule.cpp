#include "./package_schedule.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::structs {
   void package_schedule::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      subrecord.read(this->month);
      subrecord.read(this->weekday);
      subrecord.read(this->day);
      subrecord.read(this->hour);
      subrecord.read(this->minute);
      subrecord.skip_bytes(3);
      subrecord.read(this->duration);
   }
   void package_schedule::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write(this->month);
      subrecord.write(this->weekday);
      subrecord.write(this->day);
      subrecord.write(this->hour);
      subrecord.write(this->minute);
      subrecord.skip_bytes(3);
      subrecord.write(this->duration);
   }
}