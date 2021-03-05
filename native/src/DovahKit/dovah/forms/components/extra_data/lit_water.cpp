#include "lit_water.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result lit_water::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      auto& formID = this->refs.emplace_back();
      subrecord.read(formID);
      intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
         detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::reference, intfc.target_stub, formID)
      );
      return load_result::succeeded;
   }
   void lit_water::save(tes_record_writer& record, save_interface_t& intfc) {
      if (this->refs.empty())
         return;
      for (auto ref : this->refs) {
         auto& subrecord = record.open_next_subrecord(signature);
         subrecord.write(ref);
         subrecord.close();
      }
   }
   //
   /*static*/ void lit_water::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state&) {
      auto& subrecord = record.get_current_subrecord();
      form_id_t formID;
      if (subrecord.read(formID))
         uib.add_outbound_reference(formID);
   }
   basic_extra_data* lit_water::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new lit_water;
      //
      size_t size = this->refs.size();
      clone->refs.resize(size);
      for (size_t i = 0; i < size; ++i)
         clone->refs[i].set(clone_owner, this->refs[i]);
      //
      return clone;
   }
   void lit_water::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) {
      auto& list = this->refs;
      for (auto& id : list)
         id.clear_if(my_owner, target);
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [](form_reference_t& id) {
               return id == nullptr;
            }
         ),
         list.end()
      );
   }
}