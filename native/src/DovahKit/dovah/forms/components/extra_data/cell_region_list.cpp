#include "cell_region_list.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result cell_region_list::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      auto size = subrecord.size() / 4;
      this->regions.resize(size);
      for (size_t i = 0; i < size; ++i) {
         subrecord.read(this->regions[i]);
         intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
            detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::region, intfc.target_stub, this->regions[i])
               .set_cause_form_index(i)
         );
      }
      return load_result::succeeded;
   }
   void cell_region_list::save(tes_record_writer& record) {
      if (this->regions.empty())
         return;
      auto& subrecord = record.open_next_subrecord(signature);
      for (auto id : this->regions)
         subrecord.write(id);
      subrecord.close();
   }
   //
   /*static*/ void cell_region_list::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature) {
         auto count = subrecord.size() / 4;
         for (size_t i = 0; i < count; ++i) {
            form_id_t formID;
            if (subrecord.read(formID) && formID)
               uib.add_outbound_reference(formID);
         }
      }
   }
   basic_extra_data* cell_region_list::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new cell_region_list;
      //
      size_t size = this->regions.size();
      clone->regions.resize(size);
      for (size_t i = 0; i < size; ++i)
         clone->regions[i].set(clone_owner, this->regions[i]);
      //
      return clone;
   }
   void cell_region_list::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      auto& list = this->regions;
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