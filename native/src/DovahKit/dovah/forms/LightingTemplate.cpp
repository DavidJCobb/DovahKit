#include "LightingTemplate.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void LightingTemplate::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'DATA':
               this->data.load(subrecord, intfc);
               break;
            case 'DALC':
               this->directional_ambient.load(subrecord, intfc);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void LightingTemplate::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void LightingTemplate::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (LightingTemplate*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->data = this->data;
      copy->directional_ambient = this->directional_ambient;
   }
   void LightingTemplate::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         this->data.save(subrecord, intfc);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DALC');
         this->directional_ambient.save(subrecord, intfc);
         subrecord.close();
      }
   }
   void LightingTemplate::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->data = {};
      this->directional_ambient = {};
   }
   void LightingTemplate::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}