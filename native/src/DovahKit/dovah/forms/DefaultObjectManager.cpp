#include "DefaultObjectManager.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"
#include "../data/default_objects.h"
#include "../../helpers/unordered_map.h"
#include "factories/hardcoded.h"

namespace dovah::loaded_forms {
   form_stub* DefaultObjectManager::get_entry(signature_t signature) const noexcept {
      entry none;
      auto& e = cobb::unordered_map_get_if_present(this->entries, signature, none);
      return e.form.get_form_stub();
   }
   bool DefaultObjectManager::entry_is_edited(signature_t signature) const noexcept {
      entry none;
      auto& e = cobb::unordered_map_get_if_present(this->entries, signature, none);
      return e.is_active_file;
   }
   notice_code_t DefaultObjectManager::set_entry(signature_t signature, form_stub* stub) {
      auto* definition = get_default_object_definition(signature);
      if (definition) {
         if (stub && stub->formType != definition->type)
            return notice_code::default_object_rejected_for_bad_type;
      }
      auto& entry = this->entries[signature];
      entry.form.set(*this, stub);
      entry.is_active_file = true;
      if (!this->is_working_copy)
         this->stub.set_edited(true);
      if (!definition)
         return notice_code::default_object_accepted_but_unknown;
      return default_notice_code;
   }

   void DefaultObjectManager::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      bool is_active_file = intfc.is_active_file();
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DNAM':
               while (subrecord.is_in_bounds(8)) {
                  signature_t s;
                  subrecord.read_signature(s);
                  auto& entry = this->entries[s];
                  subrecord.read(entry.form);
                  entry.is_active_file = is_active_file;
               }
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void DefaultObjectManager::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'DNAM':
               while (subrecord.is_in_bounds(8)) {
                  subrecord.skip_bytes(sizeof(signature_t));
                  if (subrecord.read(formID))
                     uib.add_outbound_reference(formID);
               }
               break;
         }
      }
   }
   bool DefaultObjectManager::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (DefaultObjectManager*)out;
      //
      if (!copy->entries.empty()) {
         for (auto& pair : copy->entries) {
            if (pair.second.form)
               pair.second.form.set(*copy, nullptr);
         }
         copy->entries.clear();
      }
      for (auto& pair : this->entries) {
         auto& data = copy->entries[pair.first];
         data.is_active_file = pair.second.is_active_file;
         data.form.set(*copy, pair.second.form);
      }
      return true;
   }
   bool DefaultObjectManager::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      bool opened = false;
      for (auto& pair : this->entries) {
         auto& entry = pair.second;
         if (!entry.is_active_file)
            continue;
         if (!opened)
            record.open_next_subrecord('DNAM');
         auto& subrecord = record.get_current_subrecord();
         subrecord.write_signature(pair.first);
         subrecord.write(entry.form);
      }
      if (opened)
         record.get_current_subrecord().close();
      //
      return true;
   }
   void DefaultObjectManager::_sever_outbound_references_impl(form_stub& other) noexcept {
      for (auto& pair : this->entries)
         pair.second.form.clear_if(*this, other);
   }
   void DefaultObjectManager::_clear_impl() noexcept {
      for (auto& pair : this->entries)
         pair.second.form.set(*this, nullptr);
      this->entries.clear();
   }
}