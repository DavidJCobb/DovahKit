#include "ActorAction.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void ActorAction::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'CNAM':
               this->color.load(subrecord);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void ActorAction::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
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
   void ActorAction::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ActorAction*)out;

      copy->script_data.clone_from(this->script_data, *copy);
      copy->color = this->color;
   }
   void ActorAction::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& CNAM = record.open_next_subrecord('CNAM');
      this->color.save(CNAM);
      CNAM.close();
   }
   void ActorAction::_clear_impl() noexcept {
      this->script_data.clear(*this);
   }
}