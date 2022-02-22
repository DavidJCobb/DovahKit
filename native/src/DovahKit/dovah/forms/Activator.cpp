#include "Activator.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Activator::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
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
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->model.load(subrecord, intfc);
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
            case 'PNAM':
               this->marker_color.load(subrecord);
               break;
            case 'SNAM':
               subrecord.read(this->looping_sound);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->looping_sound)
               );
               break;
            case 'VNAM':
               subrecord.read(this->activation_sound);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->activation_sound)
               );
               break;
            case 'WNAM':
               subrecord.read(this->water_type);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::water_type, this->stub, this->water_type)
               );
               break;
            case 'RNAM':
               subrecord.read(this->activation_verb);
               break;
            case 'FNAM':
               subrecord.read(this->activator_flags);
               break;
            case 'KNAM':
               subrecord.read(this->interact_keyword);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::keyword, this->stub, this->interact_keyword)
               );
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void Activator::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t interact_keyword;
      form_id_t sound_loop;
      form_id_t sound_activate;
      form_id_t water_type;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'SNAM': // looping sound (e.g. nirnroot bell)
               subrecord.read(sound_loop);
               break;
            case 'VNAM': // activation sound
               subrecord.read(sound_activate);
               break;
            case 'WNAM': // water type, for water activators
               subrecord.read(water_type);
               break;
            case 'KNAM': // interaction keyword
               subrecord.read(interact_keyword);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
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
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'EDID': // editor ID
            case 'FULL': // displayed name
            case 'PNAM': // marker color
            case 'RNAM': // override activation prompt text
            case 'FNAM': // extra flags
               break;

         }
      }
      uib.add_outbound_reference(interact_keyword);
      uib.add_outbound_reference(sound_loop);
      uib.add_outbound_reference(sound_activate);
      uib.add_outbound_reference(water_type);
   }
   bool Activator::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (Activator*)out;
      //
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);
      copy->destruction_data.clone_from(this->destruction_data, *copy);
      copy->keywords.clone_from(this->keywords, *copy);
      copy->name = this->name;
      copy->marker_color = this->marker_color;
      copy->looping_sound.set(*copy, this->looping_sound);
      copy->activation_sound.set(*copy, this->activation_sound);
      copy->water_type.set(*copy, this->water_type);
      copy->interact_keyword.set(*copy, this->interact_keyword);
      copy->activation_verb = this->activation_verb;
      copy->activator_flags = this->activator_flags;
      return true;
   }
   bool Activator::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      this->destruction_data.save(record, intfc);
      this->keywords.save(record, intfc);
      auto& PNAM = record.open_next_subrecord('PNAM');
      this->marker_color.save(PNAM);
      PNAM.close();
      record.write_formID_subrecord('SNAM', this->looping_sound, true);
      record.write_formID_subrecord('VNAM', this->activation_sound, true);
      record.write_formID_subrecord('WNAM', this->water_type, true);
      if (!this->activation_verb.empty()) {
         auto& RNAM = record.open_next_subrecord('RNAM');
         RNAM.write(this->activation_verb);
         RNAM.close();
      }
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->activator_flags);
      FNAM.close();
      record.write_formID_subrecord('KNAM', this->interact_keyword, true);
      return true;
   }
   void Activator::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->destruction_data.sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      //
      this->looping_sound.clear_if(*this, other);
      this->activation_sound.clear_if(*this, other);
      this->water_type.clear_if(*this, other);
      this->interact_keyword.clear_if(*this, other);
   }
   void Activator::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->bounds.clear();
      this->model.clear(*this);
      this->destruction_data.clear(*this);
      this->keywords.clear(*this);
      this->name.reset();
      this->looping_sound.set(*this, nullptr);
      this->activation_sound.set(*this, nullptr);
      this->water_type.set(*this, nullptr);
      this->interact_keyword.set(*this, nullptr);
      this->activation_verb.reset();
      this->activator_flags = 0;
   }
}