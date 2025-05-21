#include "SoundOutputModel.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   #pragma region channel_output
      void SoundOutputModel::channel_output::load(tes_subrecord_reader& subrecord) {
         subrecord.read(this->left);
         subrecord.read(this->right);
         subrecord.read(this->center);
         subrecord.read(this->low_frequency_effects);
         subrecord.read(this->surround_left);
         subrecord.read(this->surround_right);
         subrecord.read(this->rear_surround_left);
         subrecord.read(this->rear_surround_right);
      }
      void SoundOutputModel::channel_output::save(tes_subrecord_writer& subrecord) {
         subrecord.write(this->left);
         subrecord.write(this->right);
         subrecord.write(this->center);
         subrecord.write(this->low_frequency_effects);
         subrecord.write(this->surround_left);
         subrecord.write(this->surround_right);
         subrecord.write(this->rear_surround_left);
         subrecord.write(this->rear_surround_right);
      }
   #pragma endregion

   void SoundOutputModel::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
               break;
            case 'NAM1':
               {
                  uint32_t coalesced;
                  if (subrecord.read(coalesced)) {
                     this->flags       = coalesced & 0xFF;
                     this->reverb_send = coalesced >> 24;
                  }
               }
               break;
            case 'FNAM': // legacy NAM1
               {
                  uint32_t coalesced;
                  if (subrecord.read(coalesced)) {
                     this->flags       = coalesced & 0xFF;
                     this->reverb_send = coalesced >> 24;

                     this->reverb_send |= 0x32;
                  }
               }
               break;
            case 'MNAM':
               subrecord.read(this->type);
               break;
            case 'SNAM': // legacy ONAM
               if (subrecord.size() >= 8) {
                  if (subrecord.size() >= 16) {
                     this->channels.stereo_l.load(subrecord);
                     this->channels.stereo_r.load(subrecord);
                  } else {
                     this->channels.mono.load(subrecord);
                  }
               }
               break;
            case 'ONAM':
               for (auto& item : this->channels.all) {
                  item.load(subrecord);
                  if (subrecord.is_at_end())
                     break;
               }
               break;
            case 'ANAM':
               subrecord.skip_bytes(4); // v-table pointer that CK32 vomits into the file by mistake
               subrecord.read(this->attenuation.distance.minimum);
               subrecord.read(this->attenuation.distance.maximum);
               for (auto& byte : this->attenuation.curve)
                  subrecord.read(byte);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void SoundOutputModel::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
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
   void SoundOutputModel::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (SoundOutputModel*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->type        = this->type;
      copy->flags       = this->flags;
      copy->reverb_send = this->reverb_send;
      copy->channels    = this->channels;
      copy->attenuation = this->attenuation;
   }
   void SoundOutputModel::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('NAM1');
         subrecord.write(this->flags);
         subrecord.write((uint16_t)0);
         subrecord.write(this->reverb_send);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('MNAM');
         subrecord.write(this->type);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('ONAM');
         for (auto& item : this->channels.all) {
            item.save(subrecord);
         }
         subrecord.close();
      }
      if (this->flags & sound_output_flag::attenuates_with_distance) {
         auto& subrecord = record.open_next_subrecord('ANAM');
         auto& src       = this->attenuation;
         subrecord.skip_bytes(4);
         subrecord.write(src.distance.minimum);
         subrecord.write(src.distance.maximum);
         for (auto c : src.curve)
            subrecord.write(c);
         subrecord.skip_bytes(3);
         subrecord.close();
      }
   }
   void SoundOutputModel::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->type        = sound_output_type::use_hrtf;
      this->flags       = 0;
      this->reverb_send = 50;
      this->channels    = {};
      this->attenuation = {};
   }
   void SoundOutputModel::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}