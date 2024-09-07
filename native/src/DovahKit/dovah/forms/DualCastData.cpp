#include "DualCastData.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void DualCastData::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
               this->bounds.load(subrecord, intfc);
               break;
            case 'DATA':
               if (auto& dst = this->projectile; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::projectile, subrecord.signature());
               }
               if (auto& dst = this->explosion; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::explosion, subrecord.signature());
               }
               if (auto& dst = this->effect_shader; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::effect_shader, subrecord.signature());
               }
               if (auto& dst = this->hit_effect_art; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::art_object, subrecord.signature());
               }
               if (auto& dst = this->impact_data_set; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::impact_data_set, subrecord.signature());
               }
               {
                  uint32_t flags = 0;
                  subrecord.read(flags);
                  this->inherit_scale.hit_effect_art = flags & 1;
                  this->inherit_scale.projecile      = flags & 2;
                  this->inherit_scale.explosion      = flags & 4;
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void DualCastData::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
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
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
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
   void DualCastData::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (DualCastData*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->projectile.set(*copy, this->projectile);
      copy->explosion.set(*copy, this->explosion);
      copy->effect_shader.set(*copy, this->effect_shader);
      copy->hit_effect_art.set(*copy, this->hit_effect_art);
      copy->impact_data_set.set(*copy, this->impact_data_set);
      copy->inherit_scale = this->inherit_scale;
   }
   void DualCastData::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->projectile);
      DATA.write(this->explosion);
      DATA.write(this->effect_shader);
      DATA.write(this->hit_effect_art);
      DATA.write(this->impact_data_set);
      {
         uint32_t flags = 0;
         if (this->inherit_scale.hit_effect_art)
            flags |= 1;
         if (this->inherit_scale.projecile)
            flags |= 2;
         if (this->inherit_scale.explosion)
            flags |= 4;
         DATA.write(flags);
      }
      DATA.close();
   }
   void DualCastData::_clear_impl() noexcept {
      this->bounds.clear();
      this->script_data.clear(*this);
      this->projectile.set(*this, nullptr);
      this->explosion.set(*this, nullptr);
      this->effect_shader.set(*this, nullptr);
      this->hit_effect_art.set(*this, nullptr);
      this->impact_data_set.set(*this, nullptr);
      this->inherit_scale = {};
   }
   void DualCastData::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->projectile.clear_if(*this, other);
      this->explosion.clear_if(*this, other);
      this->effect_shader.clear_if(*this, other);
      this->hit_effect_art.clear_if(*this, other);
      this->impact_data_set.clear_if(*this, other);
   }
}