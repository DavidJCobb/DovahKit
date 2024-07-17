#include "Global.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Global::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
               // that's responsible for loading it. However, TESGlobal doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'FLTV':
               subrecord.read(this->value);
               break;
            case 'FNAM':
               subrecord.read(this->value_type);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Global::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FNAM':
            case 'FLTV':
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void Global::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Global*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->value_type = this->value_type;
      copy->value      = this->value;
   }
   void Global::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->value_type);
      FNAM.close();
      auto& FLTV = record.open_next_subrecord('FLTV');
      FLTV.write(this->value);
      FLTV.close();
   }
   void Global::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->value_type = value_type::float32;
      this->value      = 0.0F;
   }
   void Global::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}