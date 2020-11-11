#include "DefaultObjectManager.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"
#include "factories/hardcoded.h"

namespace dovah::loaded_forms {
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
                  subrecord.read(s);
                  auto& entry = this->entries[s];
                  subrecord.read(entry.form);
                  entry.is_active_file = is_active_file;
               }
               break;
            default:
               intfc.log_load_warning(
                  file_read_warning::warn_about_unrecognized_subrecord(subrecord.signature(), *this->stub)
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
      auto copy = dynamic_cast<DefaultObjectManager*>(out);
      if (!copy)
         return false;
      assert(copy->stub);
      auto& clone_stub = *copy->stub;
      //
      if (!copy->entries.empty()) {
         for (auto& pair : copy->entries) {
            if (pair.second.form)
               pair.second.form.set(clone_stub, nullptr);
         }
         copy->entries.clear();
      }
      for (auto& pair : this->entries) {
         auto& data = copy->entries[pair.first];
         data.is_active_file = pair.second.is_active_file;
         data.form.set(clone_stub, pair.second.form);
      }
      return true;
   }
   bool DefaultObjectManager::_save_impl(tes_record_writer& record) {
      bool opened = false;
      for (auto& pair : this->entries) {
         auto& entry = pair.second;
         if (!entry.is_active_file)
            continue;
         if (!opened)
            record.open_next_subrecord('DNAM');
         auto& subrecord = record.get_current_subrecord();
         subrecord.write(pair.first);
         subrecord.write(entry.form);
      }
      if (opened)
         record.get_current_subrecord().close();
      //
      return true;
   }
   void DefaultObjectManager::_sever_outbound_references_impl(form_stub& other) noexcept {
      for (auto& pair : this->entries)
         pair.second.form.clear_if(*this->stub, other);
   }
}