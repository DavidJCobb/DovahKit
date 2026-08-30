#include "SoundDescriptor.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/sound_descriptor/sound_data_before_sound_class.h"
#include "../notices/form_load_warnings/by_form_type/sound_descriptor/sound_file_path_too_long.h"
#include "../notices/form_load_warnings/by_form_type/sound_descriptor/unrecognized_cnam.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::sound_descriptor;
   }
}

namespace dovah::loaded_forms {
   void SoundDescriptor::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);

      if (!intfc.is_winning_record)
         return;

      bool is_in_sound_def = false;
      
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;

         auto _warn_if_early_subrecord = [this, &intfc, &is_in_sound_def, &subrecord]() {
            if (is_in_sound_def)
               return;
            specific_load_warnings::sound_data_before_sound_class notice(
               this->stub,
               subrecord.signature()
            );
            intfc.log_load_warning(notice);
         };

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
               //
               // So technically, what happens is this: the "type" value is the CRC-32 hash 
               // of an internal class name. Bethesda has a hashmap that they use to find a 
               // factory object, to instantiate an instance of that class. This allows the 
               // "sound descriptor" form type to potentially hold multiple different sound 
               // classes.
               // 
               // (In practice, there's only one class: `BGSStandardSoundDef`.)
               // 
               // If a factory is found and an instance is created, the loader then opens 
               // the next subrecord and tells the instance to continue loading from there. 
               // As it happens, `BGSStandardSoundDef::Load` will just consume all of the 
               // remaining subrecords. Between that, and the fact that it's the only valid 
               // sound type, we don't need to actually reconstruct any of the behaviors of 
               // this system. We can just handle all the subrecords top-level, as if the 
               // abstraction in question didn't exist. The only concern is that we need to 
               // warn if CNAM isn't the right value.
               // 
               // Note also that any subrecords belonging to BGSStandardSoundDef would be 
               // ignored by the game if they come before CNAM. In practice, the subrecords 
               // that belong to BGSSoundDescriptorForm itself are:
               // 
               //  - EDID
               //  - VMAD
               //  - OBND (no-op)
               //  - CNAM
               //
               is_in_sound_def = true;
               if (this->type != descriptor_type::standard) {
                  specific_load_warnings::unrecognized_cnam notice(
                     this->stub,
                     (uint32_t)this->type
                  );
                  intfc.log_load_warning(notice);
               }
               break;

            #pragma region BGSStandardSoundDef
            case 'GNAM':
               _warn_if_early_subrecord();
               if (subrecord.read(this->category)) {
                  intfc.warn_if_ref_is_wrong_type(this->category, form_type::sound_category, subrecord.signature());
               }
               break;
            case 'CTDA':
               _warn_if_early_subrecord();
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;
            case 'SNAM':
               _warn_if_early_subrecord();
               if (auto& form = this->alternate_for; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            case 'ANAM':
               _warn_if_early_subrecord();
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
               _warn_if_early_subrecord();
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
               _warn_if_early_subrecord();
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
               _warn_if_early_subrecord();
               subrecord.read(this->frequency.shift);
               subrecord.read(this->frequency.variance);
               subrecord.read(this->priority);
               subrecord.read(this->db_variance);
               subrecord.read(this->static_attenuation);
               break;
            case 'ONAM':
               _warn_if_early_subrecord();
               if (auto& form = this->output_model; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_output_model, subrecord.signature());
               }
               break;
            #pragma endregion

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
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
         this->type = descriptor_type::standard;
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