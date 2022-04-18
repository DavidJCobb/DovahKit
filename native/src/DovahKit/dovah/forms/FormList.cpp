#include "FormList.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void FormList::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      form_reference_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'LNAM': // list entry
               if (subrecord.read(formID))
                  this->contents.push_back(formID);
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void FormList::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'LNAM': // list entry
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
         }
      }
   }
   bool FormList::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (FormList*)out;
      //
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      //
      size_t size = this->contents.size();
      copy->contents.resize(size);
      for (size_t i = 0; i < size; ++i)
         copy->contents[i].set(*copy, this->contents[i]);
      return true;
   }
   bool FormList::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      if (!this->bounds.is_zero()) {
         auto& OBND = record.open_next_subrecord('OBND');
         this->bounds.save(OBND, intfc);
         OBND.close();
      }
      for (auto& entry : this->contents)
         record.write_formID_subrecord('LNAM', entry);
      return true;
   }
   void FormList::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->bounds.clear();
      clear_form_reference_list(this->contents, *this);
   }
   void FormList::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      //
      std::vector<form_reference_t> replacement;
      bool edits = false;
      //
      auto& list = this->contents;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& id = list[i];
         if (id == &other) {
            if (!edits) {
               edits = true;
               replacement.reserve(size);
               replacement.insert(replacement.begin(), list.begin(), list.begin() + i);
            }
            id.set(*this, nullptr);
         } else if (edits) {
            replacement.push_back(id);
         }
      }
      if (edits) {
         list = replacement;
      }
   }
}