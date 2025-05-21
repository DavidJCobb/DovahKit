#include "ImpactData.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void ImpactData::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      
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
            case 'MODL':
            case 'MODT':
               this->model.load(subrecord, intfc);
               break;
            case 'DODT':
               if (!this->decal_data)
                  this->decal_data = new components::decal_data;
               this->decal_data->load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->effect.duration);
               subrecord.read(this->effect.orientation);
               subrecord.read(this->angle_threshold);
               subrecord.read(this->placement_radius);
               subrecord.read(this->loudness);
               {
                  uint8_t flags = 0;
                  subrecord.read(flags);
                  this->decal.enabled = flags & 1;
               }
               subrecord.read(this->impact_result);
               subrecord.skip_bytes(2);
               break;
            case 'DNAM':
               if (auto& form = this->decal.texture_sets.primary; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::texture_set, subrecord.signature());
               }
               break;
            case 'ENAM':
               if (auto& form = this->decal.texture_sets.secondary; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::texture_set, subrecord.signature());
               }
               break;
            case 'SNAM':
               if (auto& form = this->sounds[0]; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, std::array{ form_type::sound, form_type::sound_descriptor }, subrecord.signature());
               }
               break;
            case 'NAM1':
               if (auto& form = this->sounds[1]; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, std::array{ form_type::sound, form_type::sound_descriptor }, subrecord.signature());
               }
               break;
            case 'NAM2':
               if (auto& form = this->hazard; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::hazard, subrecord.signature());
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void ImpactData::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;

      form_id_t hazard;
      std::array<form_id_t, 2> sounds;
      std::array<form_id_t, 2> texture_sets;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               break;
            case 'DODT':
               break;
            case 'MODL':
            case 'MODT':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'DATA':
               break;
            case 'DNAM':
               subrecord.read(texture_sets[0]);
               break;
            case 'ENAM':
               subrecord.read(texture_sets[1]);
               break;
            case 'SNAM':
               subrecord.read(sounds[0]);
               break;
            case 'NAM1':
               subrecord.read(sounds[1]);
               break;
            case 'NAM2':
               subrecord.read(hazard);
               break;
         }
      }
      uib.add_outbound_reference(hazard);
      for (auto& id : sounds)
         uib.add_outbound_reference(id);
      for(auto& id : texture_sets)
         uib.add_outbound_reference(id);
   }
   void ImpactData::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ImpactData*)out;

      if (auto* p = this->decal_data) {
         copy->decal_data = new components::decal_data;
         *copy->decal_data = *p;
      } else {
         if (auto* q = copy->decal_data)
            delete q;
         copy->decal_data = nullptr;
      }
      copy->model.clone_from(this->model);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->angle_threshold = this->angle_threshold;
      copy->placement_radius = this->placement_radius;
      copy->effect = this->effect;
      copy->decal.enabled = this->decal.enabled;
      copy->loudness = this->loudness;

      copy->hazard.set(*copy, this->hazard);
      copy->decal.texture_sets.primary.set(*copy, this->decal.texture_sets.primary);
      copy->decal.texture_sets.secondary.set(*copy, this->decal.texture_sets.secondary);
      for (size_t i = 0; i < copy->sounds.size(); ++i)
         copy->sounds[i].set(*copy, this->sounds[i]);
   }
   void ImpactData::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      this->model.save(record, intfc, 'MODL', 'MODT');
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->effect.duration);
         subrecord.write(this->effect.orientation);
         subrecord.write(this->angle_threshold);
         subrecord.write(this->placement_radius);
         subrecord.write(this->loudness);
         {
            uint8_t flags = 0;
            if (this->decal.enabled)
               flags |= 1;
            subrecord.write(flags);
         }
         subrecord.write(this->impact_result);
         subrecord.skip_bytes(2);
         subrecord.close();
      }
      if (auto* data = this->decal_data) {
         auto& DODT = record.open_next_subrecord('DODT');
         data->save(DODT, intfc);
         DODT.close();
      }
      record.write_formID_subrecord('DNAM', this->decal.texture_sets.primary,   true);
      record.write_formID_subrecord('ENAM', this->decal.texture_sets.secondary, true);
      record.write_formID_subrecord('SNAM', this->sounds[0], true);
      record.write_formID_subrecord('NAM1', this->sounds[1], true);
      record.write_formID_subrecord('NAM2', this->hazard,    true);
   }
   void ImpactData::_clear_impl() noexcept {
      if (auto*& p = this->decal_data) {
         delete p;
         p = nullptr;
      }
      this->model.clear();
      this->script_data.clear(*this);

      this->angle_threshold = 0;
      this->placement_radius = 0;
      this->effect = {};
      this->decal.enabled = false;
      this->loudness = detection_loudness::normal;

      this->decal.texture_sets.primary.set(*this, nullptr);
      this->decal.texture_sets.secondary.set(*this, nullptr);
      for (auto& form : this->sounds)
         form.set(*this, nullptr);
      this->hazard.set(*this, nullptr);
   }
   void ImpactData::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->decal.texture_sets.primary.clear_if(*this, other);
      this->decal.texture_sets.secondary.clear_if(*this, other);
      for (auto& form : this->sounds)
         form.clear_if(*this, other);
      this->hazard.clear_if(*this, other);
   }
}