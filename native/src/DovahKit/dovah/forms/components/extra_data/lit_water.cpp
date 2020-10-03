#include "lit_water.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result lit_water::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      auto& formID = this->refs.emplace_back();
      subrecord.read(formID);
      return load_result::succeeded;
   }
   void lit_water::save(tes_record_writer& record) {
      for (auto ref : this->refs) {
         auto& subrecord = record.open_next_subrecord(signature);
         subrecord.write(ref);
         subrecord.close();
      }
   }
   //
   /*static*/ void lit_water::generate_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         stub->add_outbound_reference(formID);
   }
   basic_extra_data* lit_water::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new lit_water;
      //
      size_t size = this->refs.size();
      clone->refs.resize(size);
      for (size_t i = 0; i < size; ++i)
         clone->refs[i].set(&clone_owner, this->refs[i]);
      //
      return clone;
   }
}