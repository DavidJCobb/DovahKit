#include "Door.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Door::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      form_reference_t formID;
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
            case 'MODD':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case 'SNAM': // open sound
               if (subrecord.read(this->open_sound)) {
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->open_sound)
                  );
               }
               break;
            case 'ANAM': // close sound
               if (subrecord.read(this->close_sound)) {
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->close_sound)
                  );
               }
               break;
            case 'BNAM': // loop sound
               if (subrecord.read(this->loop_sound)) {
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->loop_sound)
                  );
               }
               break;
            case 'FNAM':
               subrecord.read(this->door_flags);
               break;
            case 'TNAM':
               if (subrecord.read(formID)) {
                  this->random_destinations.push_back(formID);
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), { form_type::cell, form_type::worldspace }, this->stub, formID)
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
   /*static*/ void Door::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t form_id;
      form_id_t sound_open;
      form_id_t sound_close;
      form_id_t sound_loop;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'SNAM':
               subrecord.read(sound_open);
               break;
            case 'ANAM':
               subrecord.read(sound_close);
               break;
            case 'BNAM':
               subrecord.read(sound_loop);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
            case 'MODD':
            case 'MOSD':
               components::model::generate_use_info(subrecord, uib);
               break;
            case 'DEST': // destruction stage header
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'TNAM':
               if (subrecord.read(form_id))
                  uib.add_outbound_reference(form_id);
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'FNAM': // flags
               break;
         }
      }
      uib.add_outbound_reference(sound_open);
      uib.add_outbound_reference(sound_close);
   }
   bool Door::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (Door*)out;
      //
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);
      {
         auto& src_opt = this->destruction_data;
         auto& dst_opt = copy->destruction_data;
         if (dst_opt.has_value()) {
            dst_opt.value().clear(*copy);
            dst_opt = {};
         }
         if (src_opt.has_value()) {
            dst_opt.emplace();
            dst_opt.value().clone_from(src_opt.value(), *copy);
         }
      }
      copy->door_flags = this->door_flags;
      copy->name   = this->name;
      copy->open_sound.set(*copy, this->open_sound);
      copy->close_sound.set(*copy, this->close_sound);
      copy->loop_sound.set(*copy, this->loop_sound);
      copy_form_reference_list(*copy, copy->random_destinations, this->random_destinations);
      //
      return true;
   }
   bool Door::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      record.write_formID_subrecord('SNAM', this->open_sound, true);
      record.write_formID_subrecord('ANAM', this->close_sound, true);
      record.write_formID_subrecord('BNAM', this->loop_sound, true);
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->door_flags);
      FNAM.close();
      for (auto& form : this->random_destinations)
         record.write_formID_subrecord('TNAM', form);
      return true;
   }
   void Door::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      //
      this->open_sound.clear_if(*this, other);
      this->close_sound.clear_if(*this, other);
      this->loop_sound.clear_if(*this, other);
      remove_form_from_reference_list(this->random_destinations, other, *this);
   }
   void Door::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->model.clear(*this);
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->name.reset();
      this->door_flags = 0;
      this->open_sound.set(*this, nullptr);
      this->close_sound.set(*this, nullptr);
      this->loop_sound.set(*this, nullptr);
      clear_form_reference_list(this->random_destinations, *this);
   }
}