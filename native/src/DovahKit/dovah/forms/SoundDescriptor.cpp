#include "SoundDescriptor.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/sound_descriptor/sound_file_path_too_long.h"
#include "../notices/form_load_warnings/by_form_type/sound_descriptor/unexpected_subrecord_after_cnam.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::sound_descriptor;
   }
}

namespace dovah::loaded_forms {
   void SoundDescriptor::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      uint32_t previous_signature = 0;
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
            case 'CNAM':
               subrecord.read(this->type);
               switch (auto sig = record.peek_next_subrecord_type()) {
                  case 0: // no next subrecord
                     break;
                  case 'GNAM':
                     break;
                  default:
                     {
                        specific_load_warnings::unexpected_subrecord_after_cnam notice(
                           this->stub,
                           sig
                        );
                        intfc.log_load_warning(notice);
                     }
                     // NOTE: Swallowing only happens for initial form load, not on-demand form load?
                     break;
               }
               break;
            case 'GNAM':
               if (subrecord.read(this->category)) {
                  intfc.warn_if_ref_is_wrong_type(this->category, form_type::sound_category, subrecord.signature());
               }
               //
               // NOTE: BGSSoundDescriptor attempts to read CNAM and GNAM both at startup and on demand.
               // The load performed at startup assumes that GNAM always comes after CNAM (i.e. the 
               // loader doesn't actually check for GNAM), so a misplaced GNAM subrecord will be missed. 
               // However, the on-demand load is, uh, actually normal, doesn't make that assumption, and 
               // will find GNAM correctly no matter where it is.
               //
               break;

            #pragma region Subrecords loaded on demand
            //
            // The game doesn't load these in TESForm::Load; rather, there's a separate member function 
            // on an associated class, BGSStandardSoundDef, which loads SNAM, GNAM, and these subrecords.
            //
            case 'CTDA':
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;
            case 'SNAM':
               if (auto& form = this->alternate_for; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            case 'ANAM':
               {
                  auto& item = this->sound_files.emplace_back();
                  if (subrecord.size() >= max_sound_file_path_length) {
                     specific_load_warnings::sound_file_path_too_long notice(
                        this->stub,
                        subrecord.size(),
                        this->sound_files.size() - 1
                     );
                     intfc.log_load_warning(notice);
                  }
                  subrecord.read(item);
               }
               break;
            case 'FNAM': // legacy
               {
                  uint32_t coalesced;
                  if (subrecord.read(coalesced)) {
                     uint8_t a = (coalesced >> 24) & 0xFF;
                     uint8_t b = (coalesced >> 16) & 0xFF;
                     uint8_t c = (coalesced >>  8) & 0xFF;
                     uint8_t d = coalesced & 0xFF;

                     this->length_characteristics.type = (loop_type)c;
                     if (d & 0x10) {
                        this->length_characteristics.type = (loop_type)(c | 0x08);
                     }
                     if (a & 0x02) {
                        this->length_characteristics.type = (loop_type)(c | 0x10);
                     }
                     if (a & 0x04) {
                        this->length_characteristics.type = (loop_type)(c | 0x20);
                     }
                     //
                     // Note that this will result in the "rumble send" byte being mangled.
                     // (Blind speculation: maybe that's why Sound Output Model disables 
                     // sound descriptors' rumble settings by default?)
                     //

                     uint8_t rumble_send = a;
                     this->length_characteristics.rumble_send.large = (rumble_send >> 4) * 7;
                     this->length_characteristics.rumble_send.small = (rumble_send & 0b1111) * 7;
                  }
               }
               break;
            case 'LNAM':
               {
                  uint32_t coalesced;
                  subrecord.read(coalesced);

                  this->length_characteristics.type = (loop_type)((coalesced >> 8) & 0xFF);

                  uint8_t rumble_send = coalesced >> 24;
                  this->length_characteristics.rumble_send.large = (rumble_send >> 4) * 7;
                  this->length_characteristics.rumble_send.small = (rumble_send & 0b1111) * 7;
               }
               break;
            case 'BNAM':
               subrecord.read(this->frequency.shift);
               subrecord.read(this->frequency.variance);
               subrecord.read(this->priority);
               subrecord.read(this->db_variance);
               subrecord.read(this->static_attenuation);
               break;
            case 'ONAM':
               if (auto& form = this->output_model; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_output_model, subrecord.signature());
               }
               break;
            #pragma endregion

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
         previous_signature = subrecord.signature();
      }
   }
   /*static*/ void SoundDescriptor::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      form_id_t alternate_for;
      form_id_t category;
      form_id_t output_model;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'SNAM':
               subrecord.read(alternate_for);
               break;
            case 'GNAM':
               subrecord.read(category);
               break;
            case 'ONAM':
               subrecord.read(output_model);
               break;
         }
      }
      uib.add_outbound_reference(alternate_for);
      uib.add_outbound_reference(category);
      uib.add_outbound_reference(output_model);
   }
   void SoundDescriptor::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (SoundDescriptor*)out;

      copy->script_data.clone_from(this->script_data, *copy);

      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);

      copy->type = this->type;
      copy->category.set(*copy, this->category);
      copy->alternate_for.set(*copy, this->alternate_for);
      copy->output_model.set(*copy, this->output_model);

      copy->sound_files = this->sound_files;
      copy->length_characteristics = this->length_characteristics;

      copy->frequency = this->frequency;
      copy->priority = this->priority;
      copy->db_variance = this->db_variance;
      copy->static_attenuation = this->static_attenuation;
   }
   void SoundDescriptor::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('CNAM');
         subrecord.write(this->type);
         subrecord.close();
      }
      record.write_formID_subrecord('GNAM', this->category);
      record.write_formID_subrecord('SNAM', this->alternate_for, true);
      for (auto& path : this->sound_files) {
         auto& subrecord = record.open_next_subrecord('ANAM');
         subrecord.write(path);
         subrecord.close();
      }
      record.write_formID_subrecord('ONAM', this->output_model);
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);
      {
         auto& src = this->length_characteristics;

         auto& subrecord = record.open_next_subrecord('LNAM');
         subrecord.write((uint8_t)0);
         subrecord.write(src.type);
         subrecord.write((uint8_t)0);
         {
            uint8_t coalesced = 0;
            coalesced = (src.rumble_send.large / 7) << 4;
            coalesced |= src.rumble_send.small / 7;
            subrecord.write(coalesced);
         }
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('BNAM');
         subrecord.write(this->frequency.shift);
         subrecord.write(this->frequency.variance);
         subrecord.write(this->priority);
         subrecord.write(this->db_variance);
         subrecord.write(this->static_attenuation);
         subrecord.close();
      }
   }
   void SoundDescriptor::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->conditions.clear(*this);

      this->alternate_for.set(*this, nullptr);
      this->category.set(*this, nullptr);
      this->output_model.set(*this, nullptr);
      this->sound_files.clear();
      this->length_characteristics = {};
      this->frequency = {};
      this->priority = 0x80;
      this->db_variance = 0;
      this->static_attenuation = 100;
   }
   void SoundDescriptor::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);

      this->alternate_for.clear_if(*this, other);
      this->category.clear_if(*this, other);
      this->output_model.clear_if(*this, other);
   }
}