#include "CameraShot.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void CameraShot::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'DATA':
               {
                  // The game memsets all of this to zero immediately upon seeing DATA, so 
                  // a truncated subrecord would still clear all the data.
                  this->action   = (camera_action)0;
                  this->location = (camera_subject)0;
                  this->target   = (camera_subject)0;
                  this->flags    = 0;
                  this->time_multipliers = { 0, 0, 0 };
                  this->minimum_time = 0;
                  this->maximum_time = 0;
                  this->target_percentage_between_actors = 0;
                  this->near_target_distance = 0;

                  subrecord.read(this->action);
                  subrecord.read(this->location);
                  subrecord.read(this->target);
                  subrecord.read(this->flags);
                  subrecord.read(this->time_multipliers.player);
                  subrecord.read(this->time_multipliers.target);
                  subrecord.read(this->time_multipliers.global);
                  subrecord.read(this->minimum_time);
                  subrecord.read(this->maximum_time);
                  subrecord.read(this->target_percentage_between_actors);
                  subrecord.read(this->near_target_distance);
               }
               break;
            case 'MNAM':
               if (auto& form = this->imagespace_modifier; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::imagespace_modifier, subrecord.signature());
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void CameraShot::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;

      form_id_t imagespace_mod;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               break;
            case 'MODL':
            case 'MODT':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'DATA':
               break;
            case 'MNAM':
               subrecord.read(imagespace_mod);
               break;
         }
      }
      uib.add_outbound_reference(imagespace_mod);
   }
   void CameraShot::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (CameraShot*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->model.clone_from(this->model);
      //
      copy->action   = this->action;
      copy->location = this->location;
      copy->target   = this->target;
      copy->flags    = this->flags;
      copy->time_multipliers = this->time_multipliers;
      copy->minimum_time = this->minimum_time;
      copy->maximum_time = this->maximum_time;
      copy->target_percentage_between_actors = this->target_percentage_between_actors;
      copy->near_target_distance = this->near_target_distance;
      copy->imagespace_modifier.set(*copy, this->imagespace_modifier);
   }
   void CameraShot::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      this->model.save(record, intfc, 'MODL', 'MODT');
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->action);
         subrecord.write(this->location);
         subrecord.write(this->target);
         subrecord.write(this->flags);
         subrecord.write(this->time_multipliers.player);
         subrecord.write(this->time_multipliers.target);
         subrecord.write(this->time_multipliers.global);
         subrecord.write(this->minimum_time);
         subrecord.write(this->maximum_time);
         subrecord.write(this->target_percentage_between_actors);
         subrecord.write(this->near_target_distance);
         subrecord.close();
      }
      record.write_formID_subrecord('MNAM', this->imagespace_modifier);
   }
   void CameraShot::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->model.clear();
      
      this->action   = (camera_action)0;
      this->location = (camera_subject)0;
      this->target   = (camera_subject)0;
      this->flags    = 0;
      this->time_multipliers = { 0, 0, 0 };
      this->minimum_time = 0;
      this->maximum_time = 0;
      this->target_percentage_between_actors = 0;
      this->near_target_distance = 0;
      this->imagespace_modifier.set(*this, nullptr);
   }
   void CameraShot::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);

      this->imagespace_modifier.clear_if(*this, other);
   }
}