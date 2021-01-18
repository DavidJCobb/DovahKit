#include "action.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result action::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->flags);
      return load_result::succeeded;
   }
   void action::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->flags);
      subrecord.close();
   }
   basic_extra_data* action::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new action;
      clone->flags = this->flags;
      return clone;
   }
}