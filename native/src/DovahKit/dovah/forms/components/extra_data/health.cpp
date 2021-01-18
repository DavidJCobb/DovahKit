#include "health.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result health::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->value);
      return load_result::succeeded;
   }
   void health::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->value);
      subrecord.close();
   }
   basic_extra_data* health::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new health;
      clone->value = this->value;
      return clone;
   }
}