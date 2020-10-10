#include "cell_region_list.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result cell_region_list::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      auto size = subrecord.size() / 4;
      this->regions.resize(size);
      for (size_t i = 0; i < size; ++i)
         subrecord.read(this->regions[i]);
      return load_result::succeeded;
   }
   void cell_region_list::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      for (auto id : this->regions)
         subrecord.write(id);
      subrecord.close();
   }
   //
   /*static*/ void cell_region_list::generate_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature) {
         auto count = subrecord.size() / 4;
         for (size_t i = 0; i < count; ++i) {
            form_id_t formID;
            if (subrecord.read(formID) && formID)
               stub->add_outbound_reference(formID);
         }
      }
   }
   basic_extra_data* cell_region_list::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new cell_region_list;
      //
      size_t size = this->regions.size();
      clone->regions.resize(size);
      for (size_t i = 0; i < size; ++i)
         clone->regions[i].set(&clone_owner, this->regions[i]);
      //
      return clone;
   }
   void cell_region_list::sever_outbound_references_to(form_stub& target, form_stub& my_owner) {
      auto& list = this->regions;
      for (auto& id : list)
         if (id == target.formID)
            id.set(&my_owner, bare_form_id_t(0));
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [](form_id_t& id) {
               return id == bare_form_id_t(0);
            }
         ),
         list.end()
      );
   }
   void cell_region_list::get_outbound_formIDs(std::vector<form_id_t*>& out) const noexcept {
      for (auto& id : this->regions)
         out.push_back(const_cast<form_id_t*>(&id));
   }
   void cell_region_list::on_after_delete() noexcept {
      this->collapse();
   }
   bool cell_region_list::is_empty() const noexcept {
      return this->regions.empty();
   }
   //
   void cell_region_list::collapse() noexcept {
      auto& list = this->regions;
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [](form_id_t& id) {
               return id == bare_form_id_t(0);
            }
         ),
         list.end()
      );
   }
}