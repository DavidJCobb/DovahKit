#include "reflector_refs.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result reflector_refs::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      auto& entry = this->entries.emplace_back();
      subrecord.read(entry.target);
      subrecord.read(entry.type);
      return load_result::succeeded;
   }
   void reflector_refs::save(tes_record_writer& record) {
      for (auto& entry : this->entries) {
         auto& subrecord = record.open_next_subrecord(signature);
         subrecord.write(entry.target);
         subrecord.write(entry.type);
         subrecord.close();
      }
   }
   //
   /*static*/ void reflector_refs::generate_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         stub->add_outbound_reference(formID);
   }
}