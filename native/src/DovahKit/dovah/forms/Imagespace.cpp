#include "Imagespace.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Imagespace::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'OBND':
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'ENAM': // legacy
               subrecord.read(this->hdr.eye_adapt_speed);
               subrecord.read(this->hdr.bloom.blur_radius);
               subrecord.read(this->hdr.bloom.threshold);
               subrecord.read(this->hdr.bloom.scale);
               subrecord.read(this->hdr.bloom.receive_threshold);
               subrecord.read(this->hdr.sunlight_scale);
               subrecord.read(this->hdr.sky_scale);
               this->hdr.white = this->hdr.bloom.receive_threshold;
               subrecord.read(this->cinematic.saturation);
               subrecord.read(this->cinematic.brightness);
               subrecord.read(this->cinematic.contrast);
               subrecord.read(this->tint.amount);
               subrecord.read(this->tint.r);
               subrecord.read(this->tint.g);
               subrecord.read(this->tint.b);
               break;
            case 'HNAM':
               subrecord.read(this->hdr.eye_adapt_speed);
               subrecord.read(this->hdr.bloom.blur_radius);
               subrecord.read(this->hdr.bloom.threshold);
               subrecord.read(this->hdr.bloom.scale);
               subrecord.read(this->hdr.bloom.receive_threshold);
               subrecord.read(this->hdr.white);
               subrecord.read(this->hdr.sunlight_scale);
               subrecord.read(this->hdr.sky_scale);
               subrecord.read(this->hdr.eye_adapt_strength);
               break;
            case 'CNAM':
               subrecord.read(this->cinematic.saturation);
               subrecord.read(this->cinematic.brightness);
               subrecord.read(this->cinematic.contrast);
               break;
            case 'TNAM':
               subrecord.read(this->tint.amount);
               subrecord.read(this->tint.r);
               subrecord.read(this->tint.g);
               subrecord.read(this->tint.b);
               break;
            case 'DNAM':
               subrecord.read(this->depth_of_field.strength);
               subrecord.read(this->depth_of_field.distance);
               subrecord.read(this->depth_of_field.range);
               subrecord.read(this->depth_of_field.radius);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Imagespace::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
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
   void Imagespace::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Imagespace*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->hdr = this->hdr;
      copy->cinematic = this->cinematic;
      copy->tint = this->tint;
      copy->depth_of_field = this->depth_of_field;
   }
   void Imagespace::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('HNAM');
         subrecord.write(this->hdr.eye_adapt_speed);
         subrecord.write(this->hdr.bloom.blur_radius);
         subrecord.write(this->hdr.bloom.threshold);
         subrecord.write(this->hdr.bloom.scale);
         subrecord.write(this->hdr.bloom.receive_threshold);
         subrecord.write(this->hdr.white);
         subrecord.write(this->hdr.sunlight_scale);
         subrecord.write(this->hdr.sky_scale);
         subrecord.write(this->hdr.eye_adapt_strength);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('CNAM');
         subrecord.write(this->cinematic.saturation);
         subrecord.write(this->cinematic.brightness);
         subrecord.write(this->cinematic.contrast);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('TNAM');
         subrecord.write(this->tint.amount);
         subrecord.write(this->tint.r);
         subrecord.write(this->tint.g);
         subrecord.write(this->tint.b);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->depth_of_field.strength);
         subrecord.write(this->depth_of_field.distance);
         subrecord.write(this->depth_of_field.range);
         subrecord.write(this->depth_of_field.radius);
         subrecord.close();
      }
   }
   void Imagespace::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->hdr = {};
      this->cinematic = {};
      this->tint = {};
      this->depth_of_field = {};
   }
   void Imagespace::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}