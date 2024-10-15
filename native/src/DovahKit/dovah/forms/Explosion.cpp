#include "Explosion.h"
#include "_common_cpp.h"
#include "../data/bound_object_form_types.h"

namespace dovah::loaded_forms {
   void Explosion::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
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
               this->bounds.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'MNAM':
               if (auto& dst = this->imagespace_modifier; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::imagespace_modifier, subrecord.signature());
               break;

            case components::enchantable::subrecord_signature_effect:
            case components::enchantable::subrecord_signature_effect_legacy: // ENAM
            case components::enchantable::subrecord_signature_charge:
            case components::enchantable::subrecord_signature_charge_legacy: // ANAM
               this->enchantable.load(subrecord, intfc);
               break;

            case 'DATA':
               if (auto& dst = this->light; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::light, subrecord.signature());
               for(auto& dst : this->sounds)
                  if (subrecord.read(dst))
                     intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               if (auto& dst = this->impact_data_set; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::impact_data_set, subrecord.signature());
               if (auto& dst = this->placed_object; subrecord.read(dst)) {
                  //
                  // The game dynamic-casts this pointer to a TESBoundObject and retains the 
                  // result, so that determines the form types that are valid here. Notably, 
                  // TESBoundObject subclasses include several form types that shouldn't be 
                  // valid base forms.
                  //
                  intfc.warn_if_ref_is_wrong_type(dst, bound_object_form_types, subrecord.signature());
               }
               if (auto& dst = this->projectile; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::projectile, subrecord.signature());
               subrecord.read(this->force);
               subrecord.read(this->damage);
               subrecord.read(this->radius);
               subrecord.read(this->imagespace_radius);
               subrecord.read(this->vertical_offset);
               subrecord.read(this->explosion_flags);
               subrecord.read(this->loudness);
               {
                  uint8_t bits = (this->explosion_flags >> 2) & 0b11;
                  switch (bits) {
                     case 0:
                        this->knockdown = knockdown_type::never;
                        break;
                     case 1:
                        this->knockdown = knockdown_type::always;
                        break;
                     case 2:
                        this->knockdown = knockdown_type::by_formula;
                        break;
                     case 3:
                        this->knockdown = knockdown_type::only_npcs;
                        break;
                  }
                  this->explosion_flags &= ~(0b11 << 2);
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Explosion::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t imagespace_mod;
      form_id_t impact_data_set;
      form_id_t light;
      form_id_t placed_object;
      form_id_t projectile;
      std::array<form_id_t, 2> sounds;
      components::enchantable::use_info_state enchantment_uib;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case components::enchantable::subrecord_signature_effect:
            case components::enchantable::subrecord_signature_effect_legacy: // ENAM
            case components::enchantable::subrecord_signature_charge:
            case components::enchantable::subrecord_signature_charge_legacy: // ANAM
               enchantment_uib.read(subrecord);
               break;
            case 'MNAM':
               subrecord.read(imagespace_mod);
               break;
            case 'DATA':
               subrecord.read(light);
               for (auto& dst : sounds)
                  subrecord.read(dst);
               subrecord.read(impact_data_set);
               subrecord.read(placed_object);
               subrecord.read(projectile);
               break;
         }
      }
      uib.add_outbound_reference(imagespace_mod);
      uib.add_outbound_reference(impact_data_set);
      uib.add_outbound_reference(light);
      uib.add_outbound_reference(placed_object);
      uib.add_outbound_reference(projectile);
      for(auto form : sounds)
         uib.add_outbound_reference(form);
      enchantment_uib.commit(uib);
   }
   void Explosion::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Explosion*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->enchantable.clone_from(this->enchantable, *copy);
      copy->model.clone_from(this->model, *copy);

      copy->name = this->name;
      copy->imagespace_modifier.set(*copy, this->imagespace_modifier);
      copy->impact_data_set.set(*copy, this->imagespace_modifier);
      copy->light.set(*copy, this->imagespace_modifier);
      copy->placed_object.set(*copy, this->imagespace_modifier);
      copy->projectile.set(*copy, this->imagespace_modifier);
      for (size_t i = 0; i < this->sounds.size(); ++i)
         copy->sounds[i].set(*copy, this->sounds[i]);

      copy->damage = this->damage;
      copy->force  = this->force;
      copy->radius = this->radius;
      copy->explosion_flags   = this->explosion_flags;
      copy->imagespace_radius = this->imagespace_radius;
      copy->vertical_offset   = this->vertical_offset;
      copy->knockdown = this->knockdown;
      copy->loudness = this->loudness;
   }
   void Explosion::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      this->enchantable.save(record, intfc);
      record.write_formID_subrecord('MNAM', this->imagespace_modifier, true);
      {
         auto& DATA = record.open_next_subrecord('DATA');
         DATA.write(this->light);
         for (auto& use : this->sounds)
            DATA.write(use);
         DATA.write(this->impact_data_set);
         DATA.write(this->placed_object);
         DATA.write(this->projectile);
         DATA.write(this->force);
         DATA.write(this->damage);
         DATA.write(this->radius);
         DATA.write(this->imagespace_radius);
         DATA.write(this->vertical_offset);
         {
            auto flags = this->explosion_flags;
            flags &= ~(0b11 << 2);
            {
               uint8_t bits = 0;
               switch (this->knockdown) {
                  case knockdown_type::never:
                     break;
                  case knockdown_type::always:
                     bits = 1;
                     break;
                  case knockdown_type::by_formula:
                     bits = 2;
                     break;
                  case knockdown_type::only_npcs:
                     bits = 3;
                     break;
               }
               flags |= (bits << 2);
            }
            DATA.write(flags);
         }
         DATA.write(this->loudness);
         DATA.close();
      }
   }
   void Explosion::_clear_impl() noexcept {
      this->bounds.clear();
      this->enchantable.clear(*this);
      this->model.clear(*this);
      this->script_data.clear(*this);

      this->light.set(*this, nullptr);
      this->imagespace_modifier.set(*this, nullptr);
      this->impact_data_set.set(*this, nullptr);
      this->placed_object.set(*this, nullptr);
      this->projectile.set(*this, nullptr);
      for (auto& use : this->sounds)
         use.set(*this, nullptr);

      this->damage = 0;
      this->force  = 0;
      this->radius = 0;
      this->explosion_flags = 0;
      this->imagespace_radius = 0;
      this->vertical_offset = 0;
      this->knockdown = knockdown_type::never;
      this->loudness = detection_loudness::normal;
   }
   void Explosion::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->enchantable.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->light.clear_if(*this, other);
      this->imagespace_modifier.clear_if(*this, other);
      this->impact_data_set.clear_if(*this, other);
      this->placed_object.clear_if(*this, other);
      this->projectile.clear_if(*this, other);
      for (auto& use : this->sounds)
         use.clear_if(*this, other);
   }
}