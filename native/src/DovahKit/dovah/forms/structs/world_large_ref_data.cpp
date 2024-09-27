#include "./world_large_ref_data.h"
#include "../_common_cpp.h"
#include "../../form_stub_use_info_builder_form_specific_data.h"

namespace dovah::loaded_forms::structs {
   void world_large_ref_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (!subrecord.is_skyrim_special()) {
         return;
      }
      if (subrecord.size() & 3) { // must be just dwords
         return;
      }
      if ((subrecord.size() / 2) & 1) { // must be an even number of dwords
         return;
      }
      while (subrecord.is_in_bounds(8)) {
         cell_id          cell;
         form_reference_t ref;
         subrecord.read(cell.y);
         subrecord.read(cell.x);
         if (subrecord.read(ref)) {
            intfc.warn_if_ref_is_wrong_type(ref, form_type::reference, subrecord.signature());
         }
         if (ref) {
            auto& list = this->cells_to_refs[cell];
            auto  it   = std::find(list.begin(), list.end(), ref);
            if (it == list.end())
               list.emplace_back().unmanaged_set(ref.get_form_stub());
         }
      }
   }
   void world_large_ref_data::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) const {
      for (auto& pair : this->cells_to_refs) {
         if (pair.second.empty())
            continue;
         subrecord.write(pair.first.y);
         subrecord.write(pair.first.x);
         for(auto& ref : pair.second)
            subrecord.write(ref);
      }
   }
   /*static*/ void world_large_ref_data::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      if (!subrecord.is_skyrim_special()) {
         return;
      }
      if (subrecord.size() & 3) { // must be just dwords
         return;
      }
      if ((subrecord.size() / 2) & 1) { // must be an even number of dwords
         return;
      }

      auto& dst_opt = uib.get_form_specific_data()->by_form_struct.world_large_ref_data;
      auto& dst     = dst_opt.has_value() ? dst_opt.value() : dst_opt.emplace();
      while (subrecord.is_in_bounds(8)) {
         cell_id   cell;
         form_id_t ref;
         subrecord.read(cell.y);
         subrecord.read(cell.x);
         if (ref) {
            auto& list = dst.cells_to_refs[cell];
            auto  it   = std::find(list.begin(), list.end(), ref);
            if (it == list.end())
               list.emplace_back(ref);
         }
      }
   }
   void world_large_ref_data::clone_from(const world_large_ref_data& original, loaded_forms::Form& my_containing_form) noexcept {
      this->clear(my_containing_form);
      for (auto& pair : original.cells_to_refs) {
         auto& dst = this->cells_to_refs[pair.first];
         copy_form_reference_list(my_containing_form, dst, pair.second);
      }
   }
   void world_large_ref_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept {
      std::vector<cell_id> lists_to_remove;

      for (auto& pair : this->cells_to_refs) {
         remove_form_from_reference_list(pair.second, target, my_containing_form);
         if (pair.second.empty())
            lists_to_remove.push_back(pair.first);
      }
      for (const auto id : lists_to_remove) {
         this->cells_to_refs.erase(id);
      }
   }
   void world_large_ref_data::clear(loaded_forms::Form& my_containing_form) {
      for (auto& pair : this->cells_to_refs) {
         clear_form_reference_list(pair.second, my_containing_form);
      }
      this->cells_to_refs.clear();
   }

   bool world_large_ref_data::empty() const noexcept {
      return this->cells_to_refs.empty();
   }
}