#include "Outfit.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Outfit::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'INAM': // list entry
               while (subrecord.read(formID)) {
                  intfc.warn_if_ref_is_wrong_type(formID, std::array{ form_type::armor, form_type::leveled_item }, subrecord.signature());
                  this->contents.push_back(formID);
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Outfit::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'INAM': // list entry
               while (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
         }
      }
   }
   void Outfit::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Outfit*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      //
      size_t size = this->contents.size();
      copy->contents.resize(size);
      for (size_t i = 0; i < size; ++i)
         copy->contents[i].set(*copy, this->contents[i]);
   }
   void Outfit::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      if (!this->contents.empty()) {
         auto& INAM = record.open_next_subrecord('INAM');
         for(auto& entry : this->contents)
            INAM.write(entry);
         INAM.close();
      }
   }
   void Outfit::_clear_impl() noexcept {
      this->script_data.clear(*this);
      clear_form_reference_list(this->contents, *this);
   }
   void Outfit::_sever_outbound_references_impl(form_stub& other) noexcept {
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