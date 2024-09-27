#include "./world_max_height_data.h"

namespace dovah::loaded_forms::structs {
   void world_max_height_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      subrecord.read(this->min.x);
      subrecord.read(this->min.y);
      subrecord.read(this->max.x);
      subrecord.read(this->max.y);
      while (subrecord.is_in_bounds(4)) {
         auto& entry = this->cells.emplace_back();
         subrecord.unchecked_read(entry.sw);
         subrecord.unchecked_read(entry.se);
         subrecord.unchecked_read(entry.nw);
         subrecord.unchecked_read(entry.ne);
      }
   }
   void world_max_height_data::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) const {
      subrecord.write(this->min.x);
      subrecord.write(this->min.y);
      subrecord.write(this->max.x);
      subrecord.write(this->max.y);
      for (const auto& entry : this->cells) {
         subrecord.write(entry.sw);
         subrecord.write(entry.se);
         subrecord.write(entry.nw);
         subrecord.write(entry.ne);
      }
   }
}