#include "patrol_ref_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result patrol_ref_data::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      switch (subrecord.signature()) {
         case signature_time:
            subrecord.read(this->idle_time);
            break;
         case signature_event:
            return load_result::requires_record;
         default:
            return load_result::unrecognized;
      }
      return load_result::succeeded;
   }
   bool patrol_ref_data::load(tes_record_reader& record, load_interface_t& intfc) {
      return this->event.load(record, intfc);
   }
   void patrol_ref_data::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& XPRD = record.open_next_subrecord(signature_time);
      XPRD.write(this->idle_time);
      XPRD.close();
      if (!this->event.empty())
         this->event.save(record);
   }
   //
   /*static*/ void patrol_ref_data::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_event)
         package_event_dialogue::generate_use_info(record, uib);
   }
   basic_extra_data* patrol_ref_data::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new patrol_ref_data;
      clone->idle_time = this->idle_time;
      clone->event.clone_from(this->event, clone_owner);
      return clone;
   }
   void patrol_ref_data::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      this->event.sever_outbound_references_to(target, my_owner);
   }
}