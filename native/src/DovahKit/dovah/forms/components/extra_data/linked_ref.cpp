#include "linked_ref.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result linked_ref::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      if (subrecord.size() >= 8)
         subrecord.read(this->keyword);
      subrecord.read(this->ref);
      return load_result::succeeded;
   }
   void linked_ref::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      if (this->keyword)
         subrecord.write(this->keyword);
      subrecord.write(this->ref);
      subrecord.close();
   }
   //
   /*static*/ void linked_ref::generate_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         stub->add_outbound_reference(formID);
      if (!subrecord.is_in_bounds(4))
         return;
      if (subrecord.read(formID) && formID)
         stub->add_outbound_reference(formID);
   }
   basic_extra_data* linked_ref::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new linked_ref;
      clone->keyword.set(&clone_owner, this->keyword);
      clone->ref.set(&clone_owner, this->ref);
      return clone;
   }
   void linked_ref::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      if (this->keyword == target.formID)
         this->keyword.set(&my_owner, nullptr);
      if (this->ref == target.formID)
         this->ref.set(&my_owner, nullptr);
   }
   void linked_ref::get_outbound_formIDs(std::vector<form_id_t*>& out) const noexcept {
      out.push_back(const_cast<form_id_t*>(&this->keyword));
      out.push_back(const_cast<form_id_t*>(&this->ref));
   }
   bool linked_ref::is_empty() const noexcept {
      return this->ref == bare_form_id_t(0);
   }
}