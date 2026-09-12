#include "./large_ref_index.h"
#include "../_common_cpp.h"
#include "../../form_stub_use_info_builder_form_specific_data.h"

namespace {
   constexpr const bool avoid_duplicate_insertions = false;
}

namespace dovah::loaded_forms::structs {
   void large_ref_index::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.size() & 3) { // must be just dwords
         return;
      }
      if ((subrecord.size() / 2) & 1) { // must be an even number of dwords
         return;
      }

      cell_grid_dword cell;
      subrecord.unchecked_read(cell.y);
      subrecord.unchecked_read(cell.x);
      uint32_t ref_count = 0;
      subrecord.unchecked_read(ref_count);
      {
         auto remaining = subrecord.size() - subrecord.offset();
         auto max_count = remaining / 8;
         if (ref_count > max_count)
            ref_count = max_count;
      }

      auto& cell_info = this->cells_to_refs[cell];
      for (size_t i = 0; i < ref_count; ++i) {
         form_reference_t ref;
         cell_grid_dword  parent_cell;
         subrecord.unchecked_read(ref);
         intfc.warn_if_ref_is_wrong_type(ref, form_type::reference, subrecord.signature());
         subrecord.unchecked_read(cell.y);
         subrecord.unchecked_read(cell.x);
         if constexpr (avoid_duplicate_insertions) {
            auto it = std::find_if(cell_info.begin(), cell_info.end(), [&ref](const auto& item) {
               return item.form == ref;
            });
            if (it != cell_info.end())
               continue;
         }
         cell_info.emplace_back(ref_info{ ref, parent_cell });
      }
   }
   void large_ref_index::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) const {
      for (const auto& pair : this->cells_to_refs) {
         if (pair.second.empty())
            continue;
         auto& subrecord = record.open_next_subrecord('RNAM');
         subrecord.write(pair.first.y);
         subrecord.write(pair.first.x);
         {
            uint32_t count = pair.second.size();
            subrecord.write(count);
         }
         for (const ref_info& ref : pair.second) {
            subrecord.write(ref.form);
            subrecord.write(ref.parent_cell_id.y);
            subrecord.write(ref.parent_cell_id.x);
         }
         subrecord.close();
      }
   }
   /*static*/ void large_ref_index::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      if (subrecord.size() & 3) { // must be just dwords
         return;
      }
      if ((subrecord.size() / 2) & 1) { // must be an even number of dwords
         return;
      }

      cell_grid_dword cell;
      subrecord.unchecked_read(cell.y);
      subrecord.unchecked_read(cell.x);
      uint32_t ref_count = 0;
      subrecord.unchecked_read(ref_count);
      {
         auto remaining = subrecord.size() - subrecord.offset();
         auto max_count = remaining / 8;
         if (ref_count > max_count)
            ref_count = max_count;
      }

      auto& dst_opt = uib.get_form_specific_data()->by_form_struct.large_ref_index;
      auto& dst     = dst_opt.has_value() ? dst_opt.value() : dst_opt.emplace();
      while (subrecord.is_in_bounds(8)) {
         cell_grid_dword cell;
         form_id_t       ref;
         subrecord.read(cell.y);
         subrecord.read(cell.x);
         if (ref) {
            auto& cell_info = dst.cells_to_refs[cell];
            if constexpr (avoid_duplicate_insertions) {
               auto it = std::find(cell_info.begin(), cell_info.end(), ref);
               if (it != cell_info.end())
                  continue;
            }
            cell_info.emplace_back(ref);
         }
      }
   }
   void large_ref_index::clone_from(const large_ref_index& original, loaded_forms::Form& my_containing_form) noexcept {
      this->clear(my_containing_form);
      for (auto& pair : original.cells_to_refs) {
         auto& src = pair.second;
         auto& dst = this->cells_to_refs[pair.first];
         dst.resize(src.size());
         for (size_t i = 0; i < src.size(); ++i) {
            dst[i].form.set(my_containing_form, src[i].form);
            dst[i].parent_cell_id = src[i].parent_cell_id;
         }
      }
   }
   void large_ref_index::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept {
      std::vector<cell_grid_dword> lists_to_remove;

      for (auto& pair : this->cells_to_refs) {
         std::erase_if(
            pair.second,
            [&target](auto& ref) {
               return ref.form.get_form_stub() == &target;
            }
         );
         if (pair.second.empty())
            lists_to_remove.push_back(pair.first);
      }
      for (const auto id : lists_to_remove) {
         this->cells_to_refs.erase(id);
      }
   }
   void large_ref_index::clear(loaded_forms::Form& my_containing_form) {
      for (auto& pair : this->cells_to_refs) {
         for (auto& ref : pair.second)
            ref.form.set(my_containing_form, nullptr);
         pair.second.clear();
      }
      this->cells_to_refs.clear();
   }

   bool large_ref_index::empty() const noexcept {
      return this->cells_to_refs.empty();
   }
}