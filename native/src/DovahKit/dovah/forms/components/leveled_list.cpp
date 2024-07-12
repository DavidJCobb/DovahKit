#include "./leveled_list.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/by_form_component/leveled_list/leading_coed_bleedthrough.h"
#include "../../notices/form_save_errors/by_form_component/leveled_list/too_many_entries.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::leveled_list;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_component::leveled_list;
   }
}

namespace dovah::loaded_forms::components {
   bool leveled_list::allows_form_type(form_type ft) const {
      if (this->_allowed_form_types.list == nullptr)
         return true;

      for (size_t i = 0; i < this->_allowed_form_types.count; ++i)
         if (this->_allowed_form_types.list[i] == ft)
            return true;
      return false;
   }
   [[nodiscard]] std::vector<form_type> leveled_list::legal_form_types() const {
      std::vector<form_type> out;
      
      const auto& src = this->_allowed_form_types;
      if (src.list && src.count) {
         out.resize(src.count);
         for (size_t i = 0; i < src.count; ++i)
            out[i] = src.list[i];
      }
      return out;
   }

   void leveled_list::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      std::optional<std::vector<form_type>> legal_form_types; // for error reporting; instantiate only if needed

      switch (subrecord.signature()) {
         case 'LVLD':
            subrecord.read(this->chance_none.percentage);
            break;
         case 'LVLF':
            subrecord.read(this->flags);
            break;
         case 'LVLG':
            subrecord.read(this->chance_none.global);
            break;
         case 'LLCT':
            {
               length_type reserve = 0;
               if (subrecord.read(reserve))
                  this->entries.reserve(reserve);
            }
            break;
         case 'LVLO':
            {
               struct {
                  form_reference_t form = {};
                  uint16_t         count = 0;
                  uint16_t         level = 0;
               } data;

               subrecord.read(data.level);
               subrecord.skip_bytes(2);
               subrecord.read(data.form);
               subrecord.read(data.count);
               subrecord.skip_bytes(2);

               this->entries.push_back(entry{
                  .count = data.count,
                  .level = data.level,
               });

               auto& new_entry  = this->entries.back();
               auto* entry_form = data.form.get_form_stub();
               new_entry.form.unmanaged_set(entry_form);

               // Warn on illegal form types:
               if (entry_form && !this->allows_form_type(entry_form->form_type)) {
                  if (!legal_form_types.has_value()) {
                     legal_form_types = this->legal_form_types();
                  }
                  intfc.warn_if_ref_is_wrong_type(new_entry.form, legal_form_types.value(), subrecord.signature());
               }
            }
            break;
         case 'COED':
            if (this->entries.empty()) {
               specific_load_warnings::leading_coed_bleedthrough notice(intfc.target_stub);
               intfc.log_load_warning(notice);
               //
               // The game would skip a COED if there were no bleedthrough possibility and 
               // the COED preceded any LVLO. We'll skip it, too.
               //
               break;
            }
            {
               auto& entry = this->entries.back();
               auto& dst   = entry.item_extra_data;
               if (dst.has_value()) {
                  //
                  // The game skips COEDs after the first due to how it associates them with 
                  // LVLOs. Basically, Bethesda keeps track of the last-loaded LVLO via a 
                  // static pointer, and when they see a COED, they write into that LVLO and 
                  // then clear that pointer.
                  //
               } else {
                  dst.emplace().load(subrecord, intfc);
               }
            }
            break;
      }
   }
   void leveled_list::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& LVLD = record.open_next_subrecord('LVLD');
      LVLD.write(this->chance_none.percentage);
      LVLD.close();
      auto& LVLF = record.open_next_subrecord('LVLF');
      LVLF.write(this->flags);
      LVLF.close();
      record.write_formID_subrecord('LVLG', this->chance_none.global, true);
      if (!this->entries.empty()) {
         if (this->entries.size() > std::numeric_limits<length_type>::max()) {
            auto notice = specific_save_errors::too_many_entries(
               *intfc.target_stub,
               this->entries.size()
            );
            intfc.throw_save_error(notice);
         }
         auto& LLCT = record.open_next_subrecord('LLCT');
         LLCT.write((uint8_t)this->entries.size());
         LLCT.close();

         for (const auto& entry : this->entries) {
            auto& LVLO = record.open_next_subrecord('LVLO');
            LVLO.write(entry.level);
            LVLO.skip_bytes(2);
            LVLO.write(entry.form);
            LVLO.write(entry.count);
            LVLO.skip_bytes(2);
            LVLO.close();
            if (auto& opt = entry.item_extra_data; opt.has_value())
               opt.value().save(record, intfc);
         }
      }
   }
   /*static*/ void leveled_list::generate_use_info(tes_subrecord_reader& subrecord, use_info_builder& uib) {
      bool      has_seen_lvlo = false;
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'LVLD':
            break;
         case 'LVLF':
            break;
         case 'LVLG':
            subrecord.read(uib.global);
            break;
         case 'LLCT':
            break;
         case 'LVLO':
            has_seen_lvlo = true;
            subrecord.read(formID);
            uib.owner.add_outbound_reference(formID);
            break;
         case 'COED':
            if (has_seen_lvlo) {
               has_seen_lvlo = false;
               structs::container_object_extra_data::generate_use_info(subrecord, uib.owner);
            }
            break;
      }
   }
   void leveled_list::clone_from(const leveled_list& original, loaded_forms::Form& my_containing_form) noexcept {
      this->_allowed_form_types = original._allowed_form_types;

      size_t t_size = this->entries.size();
      size_t o_size = original.entries.size();
      if (t_size < o_size) {
         this->entries.resize(o_size);
      }
      for (size_t i = 0; i < o_size; ++i) {
         auto& dst = this->entries[i];
         auto& src = original.entries[i];

         dst.form.set(my_containing_form, src.form);
         dst.count = src.count;
         dst.level = src.level;
         {
            auto& src_opt = src.item_extra_data;
            auto& dst_opt = dst.item_extra_data;
            if (src_opt.has_value()) {
               if (!dst_opt.has_value())
                  dst_opt.emplace();
               dst_opt.value().clone_from(src_opt.value(), my_containing_form);
            } else {
               if (dst_opt.has_value()) {
                  dst_opt.value().clear(my_containing_form);
                  dst_opt = {};
               }
            }
         }
      }
      if (t_size > o_size) {
         for (size_t i = o_size; i < t_size; ++i) {
            auto& dst = this->entries[i];
            dst.form.set(my_containing_form, nullptr);
            if (auto& opt = dst.item_extra_data; opt.has_value()) {
               opt.value().clear(my_containing_form);
               opt = {};
            }
         }
         this->entries.resize(o_size);
      }
   }
   void leveled_list::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_containing_form) noexcept {
      //
      // Also remove entries that refer to the target.
      //
      bool any_removed = false;
      for (auto& entry : this->entries) {
         if (entry.form.get_form_stub() == &target) {
            entry.form.set(my_containing_form, nullptr);
            if (auto& opt = entry.item_extra_data; opt.has_value()) {
               opt.value().clear(my_containing_form);
               opt = {};
            }
            any_removed = true;
            continue;
         }
         if (entry.form == nullptr) { // we'll end up removing these too
            if (auto& opt = entry.item_extra_data; opt.has_value()) {
               opt.value().clear(my_containing_form);
               opt = {};
            }
            any_removed = true;
            continue;
         }
         if (auto& opt = entry.item_extra_data; opt.has_value()) {
            opt.value().sever_outbound_references_to(target, my_containing_form);
         }
      }
      if (any_removed) {
         std::erase_if(
            this->entries,
            [](const entry& e) -> bool {
               return e.form == nullptr;
            }
         );
      }
   }
   void leveled_list::clear(loaded_forms::Form& my_containing_form) {
      for (auto& entry : this->entries) {
         entry.form.set(my_containing_form, nullptr);
         if (auto& opt = entry.item_extra_data; opt.has_value()) {
            opt.value().clear(my_containing_form);
            opt = {};
         }
      }
      this->entries.clear();
   }
}