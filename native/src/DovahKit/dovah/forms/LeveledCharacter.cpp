#include "LeveledCharacter.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void LeveledCharacter::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
               this->bounds.load(subrecord, intfc);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'LVLD':
            case 'LVLF':
            case 'LVLG':
            case 'LLCT':
            case 'LVLO':
            case 'COED':
               this->leveled_list_data.load(subrecord, intfc);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      this->leveled_list_data.post_load();
   }
   /*static*/ void LeveledCharacter::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      components::leveled_list::use_info_builder leveled_list_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               components::model::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'LVLD':
            case 'LVLF':
            case 'LVLG':
            case 'LLCT':
            case 'LVLO':
            case 'COED':
               components::leveled_list::generate_use_info(subrecord, leveled_list_uib);
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
      leveled_list_uib.done();
   }
   void LeveledCharacter::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (LeveledCharacter*)out;
      
      copy->bounds = this->bounds;
      copy->leveled_list_data.clone_from(this->leveled_list_data, *copy);
      copy->model.clone_from(this->model, *copy);
      copy->script_data.clone_from(this->script_data, *copy);
   }
   void LeveledCharacter::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      this->leveled_list_data.save(record, intfc);
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
   }
   void LeveledCharacter::_clear_impl() noexcept {
      this->bounds.clear();
      this->script_data.clear(*this);
      this->leveled_list_data.clear(*this);
      this->model.clear(*this);
   }
   void LeveledCharacter::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->leveled_list_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
   }
}