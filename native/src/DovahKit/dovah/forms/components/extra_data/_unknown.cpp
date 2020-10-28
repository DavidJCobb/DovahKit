#include "_unknown.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra::deprecated {
   extra_data_load_result XSED::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      if (subrecord.size() != 4) {
         uint8_t value;
         subrecord.read(value);
         this->value = value;
      } else {
         subrecord.read(this->value);
      }
      return load_result::succeeded;
   }
   void XSED::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->value);
      subrecord.close();
   }
   basic_extra_data* XSED::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new XSED;
      clone->value = this->value;
      return clone;
   }
}