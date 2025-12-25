#include "./reflector_refs.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result reflector_refs::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      auto& entry = this->entries.emplace_back();
      subrecord.read(entry.target);
      subrecord.read(entry.type);
      intfc.warn_if_ref_is_wrong_type(entry.target, form_type::reference, subrecord, { .nth_reference = this->entries.size() - 1 });
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result reflector_refs::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void reflector_refs::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      for (auto& entry : this->entries) {
         auto& subrecord = record.open_next_subrecord(signature);
         subrecord.write(entry.target);
         subrecord.write(entry.type);
         subrecord.close();
      }
   }
   
   /*static*/ void reflector_refs::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID))
         uib.add_outbound_reference(formID);
   }
   /*virtual*/ void reflector_refs::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      for (auto& item : this->entries) {
         item.target.set(my_owner, nullptr);
      }
      this->entries.clear();
   }
   /*virtual*/ void reflector_refs::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
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
   
   /*virtual*/ extra_data* reflector_refs::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new reflector_refs;
      //
      size_t size = this->entries.size();
      clone->entries.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& entry = clone->entries[i];
         auto& from = this->entries[i];
         entry.target.set(clone_owner, from.target);
         entry.type = from.type;
      }
      //
      return clone;
   }
}