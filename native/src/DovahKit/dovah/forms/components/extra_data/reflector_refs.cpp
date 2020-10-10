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
   basic_extra_data* reflector_refs::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new reflector_refs;
      //
      size_t size = this->entries.size();
      clone->entries.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& entry = clone->entries[i];
         auto& from  = this->entries[i];
         entry.target.set(&clone_owner, from.target);
         entry.type = from.type;
      }
      //
      return clone;
   }
   void reflector_refs::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      auto& list = this->entries;
      for (auto& entry : list)
         if (entry.target == target.formID)
            entry.target.set(&my_owner, bare_form_id_t(0));
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [](entry& e) {
               return e.target == bare_form_id_t(0);
            }
         ),
         list.end()
      );
   }
   void reflector_refs::get_outbound_formIDs(std::vector<form_id_t*>& out) const noexcept {
      for (auto& entry : this->entries)
         out.push_back(const_cast<form_id_t*>(&entry.target));
   }
   void reflector_refs::on_after_delete() noexcept {
      this->collapse();
   }
   bool reflector_refs::is_empty() const noexcept {
      return this->entries.empty();
   }
   //
   void reflector_refs::collapse() noexcept {
      auto& list = this->entries;
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [](entry& entry) {
               return entry.target == bare_form_id_t(0);
            }
         ),
         list.end()
      );
   }
}