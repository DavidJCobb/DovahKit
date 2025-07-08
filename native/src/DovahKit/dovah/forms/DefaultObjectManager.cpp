#include "DefaultObjectManager.h"
#include "_common_cpp.h"
#include "../data/default_objects.h"
#include "../../helpers/unordered_map.h"
#include "factories/hardcoded.h"

#include "../exceptions/default_object_assign_failed.h"

#include "../form_stub_use_info_builder_form_specific_data.h"

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
   void DefaultObjectManager::set_entry(signature_t signature, form_stub* stub) {
      auto* definition = get_default_object_definition(signature);
      if (definition) {
         if (stub && stub->form_type != definition->type) {
            exceptions::default_object_assign_failed ex;
            ex.dobj_signature   = signature;
            ex.known_definition = definition;
            ex.used_form_stub   = stub;
            throw ex;
         }
      }
      auto& entry = this->entries[signature];
      entry.form.set(*this, stub);
      entry.is_active_file = true;
      if (!this->is_working_copy)
         this->stub.set_edited(true);
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
      // DOBJ coalesces data across multiple files, so we kinda need some help here... The 
      // use info builder gives us a storage area for relatively complex arrangements of 
      // to-be-committed use info. We'll make use of that. Said storage area automatically 
      // commits any outbound uses we put inside of it, so we just need to add the uses.
      //
      auto& dst_opt = uib.get_form_specific_data()->by_form_type.default_object_manager;
      auto& dst     = dst_opt.has_value() ? dst_opt.value() : dst_opt.emplace();
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'DNAM':
               while (subrecord.is_in_bounds(8)) {
                  signature_t key;
                  form_id_t   form;
                  if (subrecord.read(key) && subrecord.read(form) && key)
                     dst.default_objects[key] = form;
               }
               break;
         }
      }
   }
   void DefaultObjectManager::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
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
   }
   void DefaultObjectManager::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      bool opened = false;
      for (auto& pair : this->entries) {
         auto& entry = pair.second;
         if (!entry.is_active_file)
            continue;
         if (!opened) {
            opened = true;
            record.open_next_subrecord('DNAM');
         }
         auto& subrecord = record.get_current_subrecord();
         subrecord.write_signature(pair.first);
         subrecord.write(entry.form);
      }
      if (opened)
         record.get_current_subrecord().close();
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