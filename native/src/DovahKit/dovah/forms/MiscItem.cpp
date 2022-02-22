#include "MiscItem.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   void MiscItem::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'ICON':
               subrecord.read(this->icon);
               break;
            case 'MICO':
               subrecord.read(this->message_icon);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               this->destruction_data.load(subrecord, intfc);
               break;
            case 'KSIZ':
            case 'KWDA':
               this->keywords.load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->value);
               subrecord.read(this->weight);
               break;
            case 'YNAM':
               if (subrecord.read(this->take_sound)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->take_sound)
                  );
               }
               break;
            case 'ZNAM':
               if (subrecord.read(this->drop_sound)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->drop_sound)
                  );
               }
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void MiscItem::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t take_sound;
      form_id_t drop_sound;
      form_id_t form_id;
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
            case 'ICON':
            case 'MICO':
               break;
            case 'DEST': // destruction stage header
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, uib);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               break;
            case 'YNAM':
               subrecord.read(take_sound);
               break;
            case 'ZNAM':
               subrecord.read(drop_sound);
               break;
         }
      }
      uib.add_outbound_reference(take_sound);
      uib.add_outbound_reference(drop_sound);
      uib.add_outbound_reference(form_id);
   }
   bool MiscItem::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (MiscItem*)out;
      //
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);
      copy->destruction_data.clone_from(this->destruction_data, *copy);
      copy->keywords.clone_from(this->keywords, *copy);
      copy->name         = this->name;
      copy->icon         = this->icon;
      copy->message_icon = this->message_icon;
      copy->drop_sound.set(*copy, this->drop_sound);
      copy->take_sound.set(*copy, this->take_sound);
      //
      return true;
   }
   bool MiscItem::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (!this->icon.empty())
         record.write_string_subrecord('ICON', this->icon);
      if (!this->message_icon.empty())
         record.write_string_subrecord('MICO', this->message_icon);
      this->destruction_data.save(record, intfc);
      record.write_formID_subrecord('YNAM', this->take_sound, true);
      record.write_formID_subrecord('ZNAM', this->drop_sound, true);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->value);
      DATA.write(this->weight);
      DATA.close();
      return true;
   }
   void MiscItem::_clear_impl() noexcept {
      this->bounds.clear();
      this->destruction_data.clear(*this);
      this->keywords.clear(*this);
      this->model.clear(*this);
      this->script_data.clear(*this);
      this->drop_sound.set(*this, nullptr);
      this->take_sound.set(*this, nullptr);
      this->value  = 0;
      this->weight = 0;
      this->name.reset();
      this->icon.clear();
      this->message_icon.clear();
   }
   void MiscItem::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->destruction_data.sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->drop_sound.clear_if(*this, other);
      this->take_sound.clear_if(*this, other);
   }
}