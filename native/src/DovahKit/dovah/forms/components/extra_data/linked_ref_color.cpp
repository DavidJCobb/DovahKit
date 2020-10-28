#include "linked_ref_color.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result linked_ref_color::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      this->start.load(subrecord);
      this->end.load(subrecord);
      return load_result::succeeded;
   }
   void linked_ref_color::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      this->start.save(subrecord);
      this->end.save(subrecord);
      subrecord.close();
   }
   basic_extra_data* linked_ref_color::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new linked_ref_color;
      clone->start = this->start;
      clone->end   = this->end;
      return clone;
   }
}