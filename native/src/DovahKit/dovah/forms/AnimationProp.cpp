#include "AnimationProp.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void AnimationProp::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      bool content_loaded = false;
      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
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
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'BNAM':
               subrecord.read(this->unload_event);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void AnimationProp::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'BNAM':
               break;
         }
      }
   }
   void AnimationProp::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (AnimationProp*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->model.clone_from(this->model, *copy);
      copy->unload_event = this->unload_event;
   }
   void AnimationProp::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      record.write_string_subrecord('BNAM', this->unload_event);
   }
   void AnimationProp::_clear_impl() noexcept {
      this->model.clear(*this);
      this->script_data.clear(*this);
      this->unload_event.clear();
   }
   void AnimationProp::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
   }
}