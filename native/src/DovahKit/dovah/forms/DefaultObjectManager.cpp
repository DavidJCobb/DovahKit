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
         if (stub && stub->form_type != definition->type)
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
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void DefaultObjectManager::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      //
      // DOBJ coalesces data across multiple files, so we kinda need a bit of a sledgehammer here... 
      // I modified the use info builder to be able to store a single untyped pointer, which forms 
      // can use for whatever purpose they need. We'll be using it for an unordered map.
      //
      using _pending_dobj_use_info_map = std::unordered_map<signature_t, form_id_t>;
      //
      if (!uib.extra_pointer)
         uib.extra_pointer = new _pending_dobj_use_info_map;
      //
      auto& entries = *(_pending_dobj_use_info_map*)uib.extra_pointer;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'DNAM':
               while (subrecord.is_in_bounds(8)) {
                  signature_t key;
                  form_id_t   form;
                  if (subrecord.read(key) && subrecord.read(form) && key)
                     entries[key] = form;
               }
               break;
         }
      }
      if (uib.is_final_file()) {
         //
         // We're loading the last source file for the Default Object Manager. Here, we'll apply the 
         // form-to-form references we've spotted across all pertinent source files, and then we'll 
         // discard the map after it's served its purpose.
         //
         for (auto& pair : entries)
            if (pair.second)
               uib.add_outbound_reference(pair.second);
         delete uib.extra_pointer;
         uib.extra_pointer = nullptr;
      }
   }
   bool DefaultObjectManager::_clone_impl(Form* out) const noexcept {
      if (out->type != form_type)
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