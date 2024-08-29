#include "EquipSlot.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void EquipSlot::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'DATA':
               subrecord.read(this->local_flags);
               break;
            case 'PNAM': // list entry
               {
                  size_t count = subrecord.size() / 4;
                  if (count < 0)
                     break;
                  for (uint32_t i = 0; i < count; i++) {
                     if (subrecord.read(formID)) {
                        this->parent_slots.push_back(formID);
                        intfc.warn_if_ref_is_wrong_type(formID, form_type::equip_slot, subrecord, { .nth_reference = i });
                     }
                  }
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void EquipSlot::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
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
            case 'PNAM': // list entry
               {
                  size_t count = subrecord.size() / 4;
                  if (count < 0)
                     break;
                  for (uint32_t i = 0; i < count; i++)
                     if (subrecord.read(formID))
                        uib.add_outbound_reference(formID);
               }
               break;
         }
      }
   }
   void EquipSlot::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (EquipSlot*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      //
      copy_form_reference_list(*copy, copy->parent_slots, this->parent_slots);
      copy->local_flags = this->local_flags;
   }
   void EquipSlot::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      if (!this->parent_slots.empty()) {
         auto& PNAM = record.open_next_subrecord('PNAM');
         for (auto& k : this->parent_slots)
            PNAM.write(k);
         PNAM.close();
      }
      {
         auto& DATA = record.open_next_subrecord('DATA');
         DATA.write(this->local_flags);
         DATA.close();
      }
   }
   void EquipSlot::_clear_impl() noexcept {
      this->script_data.clear(*this);
      clear_form_reference_list(this->parent_slots, *this);
      this->local_flags = 0;
   }
   void EquipSlot::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      remove_form_from_reference_list(this->parent_slots, other, *this);
   }
}