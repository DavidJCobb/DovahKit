#include "cell_region_list.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result cell_region_list::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      auto size = subrecord.size() / 4;
      this->regions.resize(size);
      for (size_t i = 0; i < size; ++i) {
         subrecord.read(this->regions[i]);
         intfc.warn_if_ref_is_wrong_type(this->regions[i], form_type::region, subrecord, { .nth_reference = i });
      }
      return load_result::succeeded;
   }
   void cell_region_list::save(tes_record_writer& record, save_interface_t& intfc) {
      if (this->regions.empty())
         return;
      auto& subrecord = record.open_next_subrecord(signature);
      for (auto id : this->regions)
         subrecord.write(id);
      subrecord.close();
   }
   //
   /*static*/ void cell_region_list::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature) {
         auto count = subrecord.size() / 4;
         for (size_t i = 0; i < count; ++i) {
            form_id_t formID;
            if (subrecord.read(formID))
               uib.add_outbound_reference(formID);
         }
      }
   }
   basic_extra_data* cell_region_list::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new cell_region_list;
      //
      size_t size = this->regions.size();
      clone->regions.resize(size);
      for (size_t i = 0; i < size; ++i)
         clone->regions[i].set(clone_owner, this->regions[i]);
      //
      return clone;
   }
   void cell_region_list::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) {
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