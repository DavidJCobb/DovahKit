#include "ReverbParameters.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void ReverbParameters::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'DATA':
               subrecord.read(this->decay_time);
               subrecord.read(this->hf_reference);
               subrecord.read(this->room_filter);
               subrecord.read(this->room_hf_filter);
               subrecord.read(this->reflections);
               subrecord.read(this->reverb_amp);
               subrecord.read(this->decay_hf_ratio);
               {
                  uint8_t raw;
                  subrecord.read(raw);
                  if (raw > 250)
                     raw = 250;
                  this->reflect_delay = raw * 1.2F;
               }
               subrecord.read(this->reverb_delay);
               subrecord.read(this->diffusion);
               subrecord.read(this->density);
               subrecord.read(this->unk0D);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void ReverbParameters::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
   }
   void ReverbParameters::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ReverbParameters*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->decay_time = this->decay_time;
      copy->hf_reference = this->hf_reference;
      copy->room_filter = this->room_filter;
      copy->room_hf_filter = this->room_hf_filter;
      copy->reflections = this->reflections;
      copy->reverb_amp = this->reverb_amp;
      copy->decay_hf_ratio = this->decay_hf_ratio;
      copy->reflect_delay = this->reflect_delay;
      copy->reverb_delay = this->reverb_delay;
      copy->diffusion = this->diffusion;
      copy->density = this->density;
      copy->unk0D = this->unk0D;
   }
   void ReverbParameters::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.reserve_more(0x0E);
         subrecord.write(this->decay_time);
         subrecord.write(this->hf_reference);
         subrecord.write(this->room_filter);
         subrecord.write(this->room_hf_filter);
         subrecord.write(this->reflections);
         subrecord.write(this->reverb_amp);
         subrecord.write(this->decay_hf_ratio);
         {
            uint8_t raw;
            raw = this->reflect_delay / 1.2F;
            subrecord.write(raw);
         }
         subrecord.write(this->reverb_delay);
         subrecord.write(this->diffusion);
         subrecord.write(this->density);
         subrecord.write(this->unk0D);
         subrecord.close();
      }
   }
   void ReverbParameters::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->decay_time = 0;
      this->hf_reference = 0;
      this->room_filter = 0;
      this->room_hf_filter = 0;
      this->reflections = 0;
      this->reverb_amp = 0;
      this->decay_hf_ratio = 0;
      this->reflect_delay = 0;
      this->reverb_delay = 0;
      this->diffusion = 0;
      this->density = 0;
      this->unk0D = 0;

   }
   void ReverbParameters::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}