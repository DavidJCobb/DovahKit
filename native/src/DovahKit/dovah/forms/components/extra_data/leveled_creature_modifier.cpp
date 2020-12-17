#include "leveled_creature_modifier.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result leveled_creature_modifier::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->value);
      return load_result::succeeded;
   }
   void leveled_creature_modifier::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->value);
      subrecord.close();
   }
   basic_extra_data* leveled_creature_modifier::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new leveled_creature_modifier;
      clone->value = this->value;
      return clone;
   }
}