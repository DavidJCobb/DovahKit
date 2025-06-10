#include "Sound.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Sound::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'SDSC':
               if (auto& form = this->descriptor; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            #pragma region Legacy data
               case 'FNAM':
                  if (!this->legacy.has_value())
                     this->legacy.emplace();
                  subrecord.read(this->legacy.value().path);
                  break;
               case 'SNDD':
                  if (!this->legacy.has_value())
                     this->legacy.emplace();
                  {
                     auto& dst = *this->legacy;
                     subrecord.read(dst.attenuation_distance.min);
                     subrecord.read(dst.attenuation_distance.max);
                     subrecord.read(dst.frequency_adjustment);
                     subrecord.skip_bytes(1);
                     subrecord.read(dst.flags);
                     subrecord.read(dst.static_attenuation);
                     subrecord.read(dst.times.stop);
                     subrecord.read(dst.times.start);
                     subrecord.read(dst.attenuation_curve);
                     subrecord.read(dst.reverb_attenuation_control);
                     subrecord.read(dst.priority);
                     subrecord.read(dst.unknown);
                  }
                  break;
            #pragma endregion
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Sound::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      form_id_t descriptor;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               break;
            case 'SDSC':
               subrecord.read(descriptor);
               break;
         }
      }
      uib.add_outbound_reference(descriptor);
   }
   void Sound::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Sound*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->descriptor.set(*copy, this->descriptor);
      copy->legacy = this->legacy;
   }
   void Sound::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      if (this->legacy.has_value()) {
         const auto& src = *this->legacy;
         if (!src.path.empty())
            record.write_string_subrecord('FNAM', src.path);
         auto& subrecord = record.open_next_subrecord('SNDD');
         subrecord.write(src.attenuation_distance.min);
         subrecord.write(src.attenuation_distance.max);
         subrecord.write(src.frequency_adjustment);
         subrecord.skip_bytes(1);
         subrecord.write(src.flags);
         subrecord.write(src.static_attenuation);
         subrecord.write(src.times.stop);
         subrecord.write(src.times.start);
         subrecord.write(src.attenuation_curve);
         subrecord.write(src.reverb_attenuation_control);
         subrecord.write(src.priority);
         subrecord.write(src.unknown);
         subrecord.close();
      }
      record.write_formID_subrecord('SDSC', this->descriptor);
   }
   void Sound::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->bounds.clear();
      this->descriptor.set(*this, nullptr);
      this->legacy = {};
   }
   void Sound::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->descriptor.clear_if(*this, other);
   }
}