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
               subrecord.to_string(this->name);
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
                  file_read_warning::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, *this->stub, this->looping_sound)
               );
               break;
            case 'VNAM':
               subrecord.read(this->activation_sound);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  file_read_warning::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, *this->stub, this->activation_sound)
               );
               break;
            case 'WNAM':
               subrecord.read(this->water_type);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  file_read_warning::warn_if_wrong_type(subrecord.signature(), form_type::water_type, *this->stub, this->water_type)
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
                  file_read_warning::warn_if_wrong_type(subrecord.signature(), form_type::keyword, *this->stub, this->interact_keyword)
               );
               break;
            default:
               intfc.log_load_warning(
                  file_read_warning::warn_about_unrecognized_subrecord(subrecord.signature(), *this->stub)
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
      uint32_t keywordCount = 0;
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'SNAM': // looping sound (e.g. nirnroot bell)
            case 'VNAM': // activation sound
            case 'WNAM': // water type, for water activators
            case 'KNAM': // interaction keyword
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generate_use_info(subrecord, uib);
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
   }
   bool Activator::_clone_impl(Form* out) const noexcept {
      auto copy = dynamic_cast<Activator*>(out);
      if (!copy)
         return false;
      copy->script_data.clone_from(this->script_data, *copy->stub);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy->stub);
      copy->destruction_data.clone_from(this->destruction_data, *copy->stub);
      copy->keywords.clone_from(this->keywords, *copy->stub);
      copy->name = this->name;
      copy->marker_color = this->marker_color;
      copy->looping_sound.set(*copy->stub, this->looping_sound);
      copy->activation_sound.set(*copy->stub, this->activation_sound);
      copy->water_type.set(*copy->stub, this->water_type);
      copy->interact_keyword.set(*copy->stub, this->interact_keyword);
      copy->activation_verb = this->activation_verb;
      copy->activator_flags = this->activator_flags;
      return true;
   }
   bool Activator::_save_impl(tes_record_writer& record) {
      this->script_data.save(record);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, 'MODL', 'MODT', 'MODS');
      this->destruction_data.save(record);
      this->keywords.save(record);
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
      this->script_data.sever_outbound_references_to(other, *this->stub);
      this->model.sever_outbound_references_to(other, *this->stub);
      this->destruction_data.sever_outbound_references_to(other, *this->stub);
      this->keywords.sever_outbound_references_to(other, *this->stub);
      //
      this->looping_sound.clear_if(*this->stub, other);
      this->activation_sound.clear_if(*this->stub, other);
      this->water_type.clear_if(*this->stub, other);
      this->interact_keyword.clear_if(*this->stub, other);
   }
}