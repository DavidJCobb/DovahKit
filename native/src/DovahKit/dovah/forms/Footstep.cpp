#include "Footstep.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Footstep::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
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
            case 'OBND':
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'ANAM':
               subrecord.read(this->tag);
               break;
            case 'DATA':
               if (subrecord.read(this->impact_data_set))
                  intfc.warn_if_ref_is_wrong_type(this->impact_data_set, form_type::impact_data_set, subrecord.signature());
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Footstep::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;

      form_id_t impact_data_set;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'ANAM':
               break;
            case 'TNAM':
               subrecord.read(impact_data_set);
               break;
         }
      }
      uib.add_outbound_reference(impact_data_set);
   }
   void Footstep::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Footstep*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->tag = this->tag;
      copy->impact_data_set.set(*copy, this->impact_data_set);
   }
   void Footstep::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->impact_data_set);
      DATA.close();
      record.write_string_subrecord('ANAM', this->tag);
   }
   void Footstep::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->impact_data_set.set(*this, nullptr);
      this->tag.clear();
   }
   void Footstep::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->impact_data_set.clear_if(*this, other);
   }
}