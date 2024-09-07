#include "VisualEffect.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void VisualEffect::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'DATA':
               if (auto& dst = this->art_object; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::art_object, subrecord.signature());
               }
               if (auto& dst = this->effect_shader; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::effect_shader, subrecord.signature());
               }
               {
                  uint32_t flags = 0;
                  subrecord.read(flags);
                  this->flags.rotate_to_face_target = flags & 1;
                  this->flags.camera_attached       = flags & 2;
                  this->flags.inherit_rotation      = flags & 4;
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void VisualEffect::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      form_id_t effect_shader;
      form_id_t explosion;
      form_id_t hit_effect_art;
      form_id_t impact_data_set;
      form_id_t projectile;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               subrecord.read(projectile);
               subrecord.read(explosion);
               subrecord.read(effect_shader);
               subrecord.read(hit_effect_art);
               subrecord.read(impact_data_set);
               break;
         }
      }
      uib.add_outbound_reference(projectile);
      uib.add_outbound_reference(explosion);
      uib.add_outbound_reference(effect_shader);
      uib.add_outbound_reference(hit_effect_art);
      uib.add_outbound_reference(impact_data_set);
   }
   void VisualEffect::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (VisualEffect*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->art_object.set(*copy, this->art_object);
      copy->effect_shader.set(*copy, this->effect_shader);
      copy->flags = this->flags;
   }
   void VisualEffect::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->art_object);
      DATA.write(this->effect_shader);
      {
         uint32_t flags = 0;
         if (this->flags.rotate_to_face_target)
            flags |= 1;
         if (this->flags.camera_attached)
            flags |= 2;
         if (this->flags.inherit_rotation)
            flags |= 4;
         DATA.write(flags);
      }
      DATA.close();
   }
   void VisualEffect::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->art_object.set(*this, nullptr);
      this->effect_shader.set(*this, nullptr);
      this->flags = {};
   }
   void VisualEffect::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->art_object.clear_if(*this, other);
      this->effect_shader.clear_if(*this, other);
   }
}