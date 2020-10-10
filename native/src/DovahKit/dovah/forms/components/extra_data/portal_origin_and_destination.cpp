#include "portal_origin_and_destination.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result portal_origin_and_destination::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->origin);
      subrecord.read(this->destination);
      return load_result::succeeded;
   }
   void portal_origin_and_destination::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->origin);
      subrecord.write(this->destination);
      subrecord.close();
   }
   //
   /*static*/ void portal_origin_and_destination::generate_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         stub->add_outbound_reference(formID);
      if (subrecord.read(formID) && formID)
         stub->add_outbound_reference(formID);
   }
   basic_extra_data* portal_origin_and_destination::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new portal_origin_and_destination;
      clone->origin.set(&clone_owner, this->origin);
      clone->destination.set(&clone_owner, this->destination);
      return clone;
   }
   void portal_origin_and_destination::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      if (this->origin == target.formID)
         this->origin.set(&my_owner, nullptr);
      if (this->destination == target.formID)
         this->destination.set(&my_owner, nullptr);
   }
   void portal_origin_and_destination::get_outbound_formIDs(std::vector<form_id_t*>& out) const noexcept {
      out.push_back(const_cast<form_id_t*>(&this->origin));
      out.push_back(const_cast<form_id_t*>(&this->destination));
   }
}