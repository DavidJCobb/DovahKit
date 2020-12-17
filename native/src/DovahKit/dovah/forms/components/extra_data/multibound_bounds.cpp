#include "multibound_bounds.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result multibound_bounds::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->halfwidths.x);
      subrecord.read(this->halfwidths.y);
      subrecord.read(this->halfwidths.z);
      return load_result::succeeded;
   }
   void multibound_bounds::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->halfwidths.x);
      subrecord.write(this->halfwidths.y);
      subrecord.write(this->halfwidths.z);
      subrecord.close();
   }
   basic_extra_data* multibound_bounds::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new multibound_bounds;
      clone->halfwidths = this->halfwidths;
      return clone;
   }
}