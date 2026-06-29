#include "Script.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Script::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            default:
               if (this->legacy_script.load(subrecord, intfc))
                  break;
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Script::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      components::legacy_script::use_info_state legacy_state;
      
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            default:
               if (legacy_state.load(subrecord))
                  break;
               // else unrecognized
               break;
         }
      }

      uib.add_outbound_reference(legacy_state.parent_quest);
      for (auto id : legacy_state.referenced_objects)
         uib.add_outbound_reference(id);
   }
   void Script::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Script*)out;

      copy->script_data.clone_from(this->script_data, *copy);
      copy->legacy_script.clone_from(this->legacy_script, *copy);
   }
   void Script::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      this->legacy_script.save(record, intfc);
   }
   void Script::_clear_impl() noexcept {
      this->script_data.clear(*this);
   }
   void Script::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->legacy_script.sever_outbound_references_to(other, *this);
   }
}