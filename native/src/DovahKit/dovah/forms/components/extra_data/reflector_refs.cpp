#include "reflector_refs.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result reflector_refs::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      auto& entry = this->entries.emplace_back();
      subrecord.read(entry.target);
      subrecord.read(entry.type);
      intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
         detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, intfc.target_stub, entry.target)
            .set_subrecord_index(this->entries.size() - 1)
      );
      return load_result::succeeded;
   }
   void reflector_refs::save(tes_record_writer& record) {
      if (this->entries.empty())
         return;
      for (auto& entry : this->entries) {
         auto& subrecord = record.open_next_subrecord(signature);
         subrecord.write(entry.target);
         subrecord.write(entry.type);
         subrecord.close();
      }
   }
   //
   /*static*/ void reflector_refs::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID) && formID)
         uib.add_outbound_reference(formID);
   }
   basic_extra_data* reflector_refs::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new reflector_refs;
      //
      size_t size = this->entries.size();
      clone->entries.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& entry = clone->entries[i];
         auto& from  = this->entries[i];
         entry.target.set(clone_owner, from.target);
         entry.type = from.type;
      }
      //
      return clone;
   }
   void reflector_refs::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      auto& list = this->entries;
      for (auto& entry : list)
         entry.target.clear_if(my_owner, target);
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [](entry& e) {
               return e.target == nullptr;
            }
         ),
         list.end()
      );
   }
}