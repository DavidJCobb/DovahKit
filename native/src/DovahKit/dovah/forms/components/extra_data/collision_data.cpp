#include "collision_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result collision_data::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->layer);
      return load_result::succeeded;
   }
   void collision_data::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->layer);
      subrecord.close();
   }
   basic_extra_data* collision_data::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new collision_data;
      clone->layer = this->layer;
      return clone;
   }
}