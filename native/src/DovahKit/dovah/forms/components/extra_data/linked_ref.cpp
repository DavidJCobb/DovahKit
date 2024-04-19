#include "linked_ref.h"
#include "../../_common_cpp.h"
#include "_use_info.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result linked_ref::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      if (subrecord.size() >= 8) {
         subrecord.read(this->keyword);
         intfc.warn_if_ref_is_wrong_type(this->keyword, form_type::keyword, subrecord.signature());
      }
      subrecord.read(this->ref);
      intfc.warn_if_ref_is_wrong_type(this->ref, form_type::reference, subrecord.signature());
      return load_result::succeeded;
   }
   void linked_ref::save(tes_record_writer& record, save_interface_t& intfc) {
      if (!this->ref)
         return;
      auto& subrecord = record.open_next_subrecord(signature);
      if (this->keyword)
         subrecord.write(this->keyword);
      subrecord.write(this->ref);
      subrecord.close();
   }
   //
   /*static*/ void linked_ref::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.is_in_bounds(8)) {
         subrecord.read(state.by_name.linked_ref.keyword);
         subrecord.read(state.by_name.linked_ref.ref);
         return;
      }
      subrecord.read(state.by_name.linked_ref.ref);
   }
   basic_extra_data* linked_ref::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new linked_ref;
      clone->keyword.set(clone_owner, this->keyword);
      clone->ref.set(clone_owner, this->ref);
      return clone;
   }
   void linked_ref::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) {
      this->keyword.clear_if(my_owner, target);
      this->ref.clear_if(my_owner, target);
   }
}