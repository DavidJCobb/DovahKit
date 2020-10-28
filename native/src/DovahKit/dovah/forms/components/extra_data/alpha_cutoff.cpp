#include "alpha_cutoff.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result alpha_cutoff::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->cutoff);
      subrecord.read(this->base);
      return load_result::succeeded;
   }
   void alpha_cutoff::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->cutoff);
      subrecord.write(this->base);
      subrecord.close();
   }
   basic_extra_data* alpha_cutoff::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new alpha_cutoff;
      clone->cutoff = this->cutoff;
      clone->base   = this->base;
      return clone;
   }
}