#include "magic_effect_list.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/by_form_component/magic_effect_list/expected_effect_item_subrecord.h"
#include "../../notices/form_load_warnings/by_form_component/magic_effect_list/misplaced_effect_item_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::magic_effect_list;
   }
}

namespace dovah::loaded_forms::components {
   void magic_effect_list::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      auto& subrecord = record.get_current_subrecord();
      auto  signature = subrecord.signature();
      switch (signature) {
         case subrecord_signature_effect:
         case subrecord_signature_details:
         case 'CTDA':
            break;
         default: // invalid
            assert(false && "Why was magic_effect_list::load called on a subrecord it's not built to handle?");
      }
      if (signature == subrecord_signature_effect) {
         auto& dst = this->items.emplace_back();
         if (auto& form = dst.effect; subrecord.read(form)) {
            intfc.warn_if_ref_is_wrong_type(form, form_type::magic_effect, subrecord);
         }
         //
         // The game blindly assumes that the next subrecord is EFIT.
         //
         auto& next = record.next_subrecord();
         if (next.signature() == 'CTDA') {
            dst.conditions.read_next(record, intfc);
            return;
         }
         if (next.signature() != subrecord_signature_details) {
            specific_load_warnings::expected_effect_item_subrecord notice(
               const_cast<form_stub&>(intfc.target_stub),
               next.signature()
            );
            intfc.log_load_warning(notice);
         }
         next.read(dst.magnitude);
         next.read(dst.area);
         next.read(dst.duration);
         return;
      }
      if (signature == subrecord_signature_details) {
         specific_load_warnings::misplaced_effect_item_subrecord notice(
            const_cast<form_stub&>(intfc.target_stub)
         );
         intfc.log_load_warning(notice);
         //
         if (this->items.empty()) {
            this->items.emplace_back();
         }
         auto& dst = this->items.back();
         subrecord.read(dst.magnitude);
         subrecord.read(dst.area);
         subrecord.read(dst.duration);
         return;
      }
      if (signature == 'CTDA') {
         if (this->items.empty()) {
            this->items.emplace_back();
         }
         this->items.back().conditions.read_next(record, intfc);
         return;
      }
   }
   void magic_effect_list::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      for (auto& item : this->items) {
         record.write_formID_subrecord('EFID', item.effect);
         {
            auto& subrecord = record.open_next_subrecord('EFIT');
            subrecord.write(item.magnitude);
            subrecord.write(item.area);
            subrecord.write(item.duration);
            subrecord.close();
         }
         for (auto& cnd : item.conditions) {
            cnd.save(record, intfc);
         }
      }
   }
   void magic_effect_list::clear(loaded_forms::Form& my_owner) noexcept {
      for (auto& item : this->items) {
         item.effect.set(my_owner, nullptr);
         item.conditions.clear(my_owner);
      }
      this->items.clear();
   }
   void magic_effect_list::clone_from(const magic_effect_list& other, loaded_forms::Form& my_owner) noexcept {
      this->clear(my_owner);
      size_t size = other.items.size();
      this->items.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_item = other.items[i];
         auto& dst_item = this->items[i];
         dst_item.effect.set(my_owner, src_item.effect);
         dst_item.area      = src_item.area;
         dst_item.duration  = src_item.duration;
         dst_item.magnitude = src_item.magnitude;
         dst_item.conditions.append_all_of(my_owner, src_item.conditions);
      }
   }
   void magic_effect_list::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      bool remove_some = false;
      for (auto& item : this->items) {
         if (item.effect.get_form_stub() == &target) {
            item.effect.set(my_owner, nullptr);
            item.conditions.clear(my_owner);
            remove_some = true;
            continue;
         }
         for (auto& cnd : item.conditions)
            cnd.sever_outbound_references_to(target, my_owner);
      }
      if (remove_some) {
         std::erase_if(this->items, [](const auto& item) {
            return item.effect == nullptr;
         });
      }
   }

   void magic_effect_list::use_info_state::read(tes_record_reader& record) {
      auto& subrecord = record.get_current_subrecord();
      auto  signature = subrecord.signature();
      switch (signature) {
         case subrecord_signature_effect:
         case subrecord_signature_details:
            break;
         default: // invalid
            assert(false && "Why was magic_effect_list::use_info_state::read called on a subrecord it's not built to handle?");
      }
      //
      form_id_t form_id;
      if (signature == subrecord_signature_effect) {
         if (subrecord.read(form_id) && form_id) {
            this->effect_forms.push_back(form_id);
         }
         auto& next = record.next_subrecord();
         return;
      }
   }
   void magic_effect_list::use_info_state::commit(form_stub_use_info_builder& uib) {
      for (auto id : this->effect_forms)
         uib.add_outbound_reference(id);
   }
}