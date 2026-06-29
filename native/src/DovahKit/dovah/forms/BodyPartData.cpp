#include "BodyPartData.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   #pragma region BodyPartData::part
      bool BodyPartData::part::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
         bool  is_old    = record.version() < 2;
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == 'BPTN') {
            subrecord.read(this->name);
            if (!record.next_subrecord())
               return false;
         }

         if (subrecord.signature() == 'PNAM') {
            subrecord.read(this->pose_matching);
            if (!record.next_subrecord())
               return false;
         }

         if (subrecord.signature() != 'BPNN')
            return false;
         subrecord.read(this->nodes.part);
         if (!record.next_subrecord())
            return false;

         if (subrecord.signature() != 'BPNT')
            return false;
         subrecord.read(this->nodes.vats_target);
         if (!record.next_subrecord())
            return false;

         if (subrecord.signature() != 'BPNI')
            return false;
         subrecord.read(this->nodes.ik_start);
         if (!record.next_subrecord())
            return false;

         if (subrecord.signature() != 'BPND')
            return false;
         {
            subrecord.read(this->combat.damage_mult);
            subrecord.read(this->flags);
            subrecord.read(this->limb);
            subrecord.read(this->combat.health_percent);
            subrecord.read(this->combat.actor_value_id);
            subrecord.read(this->combat.chance_to_hit);
            subrecord.read(this->gore.explodable.chance);
            subrecord.read(this->gore.explodable.debris_count);
            if (auto& dst = this->gore.explodable.debris; subrecord.read(dst))
               intfc.warn_if_ref_is_wrong_type(dst, form_type::debris, subrecord.signature());
            if (auto& dst = this->gore.explodable.explosion; subrecord.read(dst))
               intfc.warn_if_ref_is_wrong_type(dst, form_type::explosion, subrecord.signature());
            subrecord.read(this->headtracking_max_angle);
            subrecord.read(this->gore.explodable.debris_scale);
            subrecord.read(this->gore.severable.debris_count);
            if (auto& dst = this->gore.severable.debris; subrecord.read(dst))
               intfc.warn_if_ref_is_wrong_type(dst, form_type::debris, subrecord.signature());
            if (auto& dst = this->gore.severable.explosion; subrecord.read(dst))
               intfc.warn_if_ref_is_wrong_type(dst, form_type::explosion, subrecord.signature());
            subrecord.read(this->gore.severable.debris_scale);
            subrecord.read(this->gore.effect_positioning.pos.x);
            subrecord.read(this->gore.effect_positioning.pos.y);
            subrecord.read(this->gore.effect_positioning.pos.z);
            subrecord.read(this->gore.effect_positioning.rot.x);
            subrecord.read(this->gore.effect_positioning.rot.y);
            subrecord.read(this->gore.effect_positioning.rot.z);
            if (auto& dst = this->gore.severable.impact_data_set; subrecord.read(dst))
               intfc.warn_if_ref_is_wrong_type(dst, form_type::impact_data_set, subrecord.signature());
            if (auto& dst = this->gore.explodable.impact_data_set; subrecord.read(dst))
               intfc.warn_if_ref_is_wrong_type(dst, form_type::impact_data_set, subrecord.signature());
            subrecord.read(this->gore.severable.decal_count);
            subrecord.read(this->gore.explodable.decal_count);
            subrecord.skip_bytes(2); // assumed padding
            subrecord.read(this->gore.explodable.limb_replacement.scale);
            if (auto& v = this->gore.explodable.limb_replacement.scale; v == 0.0F)
               v = 0.1F;
            if (auto& v = this->gore.severable.debris_scale; v == 0.0F)
               v = 0.1F;
            if (auto& v = this->gore.explodable.debris_scale; v == 0.0F)
               v = 0.1F;
         }
         if (!record.next_subrecord())
            return false;

         if (subrecord.signature() != 'NAM1')
            return false;
         this->gore.explodable.limb_replacement.model.load_model_path(subrecord, intfc);
         if (record.version() < 3) {
            if (record.next_subrecord() && subrecord.signature() == 'NAM2') {
               //
               // The game reads 'NAM2' but then immediately discards it.
               //
               if (record.next_subrecord() && subrecord.signature() == 'NAM3') {
                  //
                  // The game reads 'NAM3' but then immediately discards it.
                  //
               } else {
                  return is_old;
               }
            } else {
               return is_old;
            }
         }
         if (record.next_subrecord() && subrecord.signature() != 'NAM4')
            return is_old;
         subrecord.read(this->nodes.gore_effect);
         if (record.next_subrecord()) {
            if (subrecord.signature() == 'NAM5') {
               this->gore.explodable.limb_replacement.model.load_precached_info(subrecord, intfc);
            }
         }
         return true;
      }
      void BodyPartData::part::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
         {
            auto& subrecord = record.open_next_subrecord('BPTN');
            subrecord.write(this->name);
            subrecord.close();
         }
         if (!this->pose_matching.empty())
            record.write_string_subrecord('PNAM', this->pose_matching);
         record.write_string_subrecord('BPNN', this->nodes.part);
         record.write_string_subrecord('BPNT', this->nodes.vats_target);
         record.write_string_subrecord('BPNI', this->nodes.ik_start);
         {
            auto& subrecord = record.open_next_subrecord('BPND');
            subrecord.write(this->combat.damage_mult);
            subrecord.write(this->flags);
            subrecord.write(this->limb);
            subrecord.write(this->combat.health_percent);
            subrecord.write(this->combat.actor_value_id);
            subrecord.write(this->combat.chance_to_hit);
            subrecord.write(this->gore.explodable.chance);
            subrecord.write(this->gore.explodable.debris_count);
            subrecord.write(this->gore.explodable.debris);
            subrecord.write(this->gore.explodable.explosion);
            subrecord.write(this->headtracking_max_angle);
            subrecord.write(this->gore.explodable.debris_scale);
            subrecord.write(this->gore.severable.debris_count);
            subrecord.write(this->gore.severable.debris);
            subrecord.write(this->gore.severable.explosion);
            subrecord.write(this->gore.severable.debris_scale);
            subrecord.write(this->gore.effect_positioning.pos.x);
            subrecord.write(this->gore.effect_positioning.pos.y);
            subrecord.write(this->gore.effect_positioning.pos.z);
            subrecord.write(this->gore.effect_positioning.rot.x);
            subrecord.write(this->gore.effect_positioning.rot.y);
            subrecord.write(this->gore.effect_positioning.rot.z);
            subrecord.write(this->gore.severable.impact_data_set);
            subrecord.write(this->gore.explodable.impact_data_set);
            subrecord.write(this->gore.severable.decal_count);
            subrecord.write(this->gore.explodable.decal_count);
            subrecord.skip_bytes(2); // assumed padding
            subrecord.write(this->gore.explodable.limb_replacement.scale);
            subrecord.close();
         }
         record.write_string_subrecord('NAM1', this->gore.explodable.limb_replacement.model.model_path);
         record.write_string_subrecord('NAM4', this->nodes.gore_effect);
         {
            auto& subrecord = record.open_next_subrecord('NAM5');
            this->gore.explodable.limb_replacement.model.save_precached_info(subrecord, intfc);
            subrecord.close();
         }
      }
      void BodyPartData::part::clone_from(BodyPartData& owner, const part& src) {
         #define ASSIGN_FORM(_field) this->_field.set(owner, src._field);
         #define ASSIGN_VALUE(_field) this->_field = src._field;

         ASSIGN_VALUE(name);
         ASSIGN_VALUE(flags);
         ASSIGN_VALUE(limb);
         ASSIGN_VALUE(combat);
         ASSIGN_VALUE(headtracking_max_angle);
         ASSIGN_VALUE(gore.effect_positioning);
         {
            ASSIGN_FORM(gore.explodable.debris);
            ASSIGN_FORM(gore.explodable.explosion);
            ASSIGN_FORM(gore.explodable.impact_data_set);
            ASSIGN_VALUE(gore.explodable.chance);
            ASSIGN_VALUE(gore.explodable.debris_count);
            ASSIGN_VALUE(gore.explodable.debris_scale);
            ASSIGN_VALUE(gore.explodable.decal_count);
            this->gore.explodable.limb_replacement.model.clone_from(src.gore.explodable.limb_replacement.model);
            ASSIGN_VALUE(gore.explodable.limb_replacement.scale);
         }
         {
            ASSIGN_FORM(gore.severable.debris);
            ASSIGN_FORM(gore.severable.explosion);
            ASSIGN_FORM(gore.severable.impact_data_set);
            ASSIGN_VALUE(gore.severable.debris_count);
            ASSIGN_VALUE(gore.severable.debris_scale);
            ASSIGN_VALUE(gore.severable.decal_count);
         }
         ASSIGN_VALUE(nodes);
         ASSIGN_VALUE(pose_matching);

         #undef ASSIGN_FORM
         #undef ASSIGN_VALUE
      }
      void BodyPartData::part::clear(BodyPartData& owner) {
         this->name.reset();
         this->flags = 0;
         this->limb = {};
         this->combat = {};
         this->headtracking_max_angle = 0.0F;
         this->gore.effect_positioning = {};
         {
            auto& dst = this->gore.explodable;
            dst.debris.set(owner, nullptr);
            dst.explosion.set(owner, nullptr);
            dst.impact_data_set.set(owner, nullptr);

            dst.chance = 0;
            dst.debris_count = 0;
            dst.debris_scale = 1.0F;
            dst.decal_count = 0;
            dst.limb_replacement.model.clear();
            dst.limb_replacement.scale = 1.0F;
         }
         {
            auto& dst = this->gore.severable;
            dst.debris.set(owner, nullptr);
            dst.explosion.set(owner, nullptr);
            dst.impact_data_set.set(owner, nullptr);

            dst.debris_count = 0;
            dst.debris_scale = 1.0F;
            dst.decal_count = 0;
         }
         this->nodes = {};
         this->pose_matching = {};
      }
      void BodyPartData::part::sever_outbound_references_to(BodyPartData& owner, form_stub& other) noexcept {
         this->gore.explodable.debris.clear_if(owner, other);
         this->gore.explodable.explosion.clear_if(owner, other);
         this->gore.severable.debris.clear_if(owner, other);
         this->gore.severable.explosion.clear_if(owner, other);
         this->gore.severable.impact_data_set.clear_if(owner, other);
         this->gore.explodable.impact_data_set.clear_if(owner, other);
      }
   #pragma endregion
   #pragma region BodyPartData::part_use_info
      bool BodyPartData::part_use_info::load(tes_record_reader& record) {
         bool  is_old    = record.version() < 2;
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == 'BPTN') {
            if (!record.next_subrecord())
               return false;
         }

         if (subrecord.signature() == 'PNAM') {
            if (!record.next_subrecord())
               return false;
         }

         if (subrecord.signature() != 'BPNN')
            return false;
         if (!record.next_subrecord())
            return false;

         if (subrecord.signature() != 'BPNT')
            return false;
         if (!record.next_subrecord())
            return false;

         if (subrecord.signature() != 'BPNI')
            return false;
         if (!record.next_subrecord())
            return false;

         if (subrecord.signature() != 'BPND')
            return false;
         {
            #define SIZEOF(path) sizeof(std::declval<part>().path)

            subrecord.skip_bytes(
               SIZEOF(combat.damage_mult) +
               SIZEOF(flags)
            );
            subrecord.read(this->limb);
            subrecord.skip_bytes(
               SIZEOF(combat.health_percent) +
               SIZEOF(combat.actor_value_id) +
               SIZEOF(combat.chance_to_hit) +
               SIZEOF(gore.explodable.chance) +
               SIZEOF(gore.explodable.debris_count)
            );
            subrecord.read(this->gore.explodable.debris);
            subrecord.read(this->gore.explodable.explosion);
            subrecord.skip_bytes(
               SIZEOF(headtracking_max_angle) +
               SIZEOF(gore.explodable.debris_scale) +
               SIZEOF(gore.severable.debris_count)
            );
            subrecord.read(this->gore.severable.debris);
            subrecord.read(this->gore.severable.explosion);
            subrecord.skip_bytes(
               SIZEOF(gore.severable.debris_scale) +
               SIZEOF(gore.effect_positioning.pos.x) +
               SIZEOF(gore.effect_positioning.pos.y) +
               SIZEOF(gore.effect_positioning.pos.z) +
               SIZEOF(gore.effect_positioning.rot.x) +
               SIZEOF(gore.effect_positioning.rot.y) +
               SIZEOF(gore.effect_positioning.rot.z)
            );
            subrecord.read(this->gore.severable.impact_data_set);
            subrecord.read(this->gore.explodable.impact_data_set);
            subrecord.skip_bytes(
               SIZEOF(gore.severable.decal_count) +
               SIZEOF(gore.explodable.decal_count) +
               2 +
               SIZEOF(gore.explodable.limb_replacement.scale)
            );

            #undef SIZEOF
         }
         if (!record.next_subrecord())
            return false;

         if (subrecord.signature() != 'NAM1') {
            return false;
         }
         if (record.version() < 3) {
            if (record.next_subrecord() && subrecord.signature() == 'NAM2') {
               //
               // The game reads 'NAM2' but then immediately discards it.
               //
               if (record.next_subrecord() && subrecord.signature() == 'NAM3') {
                  //
                  // The game reads 'NAM3' but then immediately discards it.
                  //
               } else {
                  return is_old;
               }
            } else {
               return is_old;
            }
         }
         if (record.next_subrecord() && subrecord.signature() != 'NAM4')
            return is_old;
         record.next_subrecord(); // open NAM5 if present
         return true;
      }
   #pragma endregion

   void BodyPartData::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      bool  already_in_next_subrecord = false;
      auto& subrecord = record.get_current_subrecord();
      while ((already_in_next_subrecord && record.get_current_subrecord()) || record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         already_in_next_subrecord = false;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'RAGA':
               if (auto& dst = this->ragdoll; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::ragdoll, subrecord.signature());
               break;
            case 'OBND':
               // no-op
               break;
            case 'BPNN':
            case 'BPTN':
            case 'PNAM':
               {
                  part temporary;
                  if (temporary.load(record, intfc)) {
                     size_t limb_id = (size_t)temporary.limb;
                     if (limb_id < limbs_count) {
                        auto& dst = this->parts[limb_id];
                        dst.clear(*this);
                        dst = std::move(temporary);
                     }
                     switch (subrecord.signature()) {
                        case 'BPNN':
                        case 'BPTN':
                        case 'PNAM':
                        case 'RAGA':
                           already_in_next_subrecord = true;
                           break;
                     }
                  }
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void BodyPartData::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      std::array<part_use_info, limbs_count> parts;
      form_id_t ragdoll = {};

      bool  already_in_next_subrecord = false;
      auto& subrecord = record.get_current_subrecord();
      while ((already_in_next_subrecord && record.get_current_subrecord()) || record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         already_in_next_subrecord = false;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'RAGA':
               subrecord.read(ragdoll);
               break;
            case 'BPNN':
            case 'BPTN':
            case 'PNAM':
               {
                  part_use_info temporary;
                  if (temporary.load(record)) {
                     size_t limb_id = (size_t)temporary.limb;
                     if (limb_id < limbs_count) {
                        parts[limb_id] = temporary;
                     }
                     switch (subrecord.signature()) {
                        case 'BPNN':
                        case 'BPTN':
                        case 'PNAM':
                        case 'RAGA':
                           already_in_next_subrecord = true;
                           break;
                     }
                  }
               }
               break;
         }
      }
      uib.add_outbound_reference(ragdoll);
      for (auto& p : parts) {
         uib.add_outbound_reference(p.gore.explodable.debris);
         uib.add_outbound_reference(p.gore.explodable.explosion);
         uib.add_outbound_reference(p.gore.explodable.impact_data_set);
         uib.add_outbound_reference(p.gore.severable.debris);
         uib.add_outbound_reference(p.gore.severable.explosion);
         uib.add_outbound_reference(p.gore.severable.impact_data_set);
      }
   }
   void BodyPartData::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (BodyPartData*)out;

      copy->model.clone_from(this->model);
      copy->ragdoll.set(*copy, this->ragdoll);
      for (size_t i = 0; i < this->parts.size(); ++i) {
         copy->parts[i].clone_from(*copy, this->parts[i]);
      }
   }
   void BodyPartData::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->model.save(record, intfc, 'MODL', 'MODT');
      for (auto& part : this->parts)
         part.save(record, intfc);
      record.write_formID_subrecord('RAGA', this->ragdoll, true);
   }
   void BodyPartData::_clear_impl() noexcept {
      this->model.clear();
      this->ragdoll.set(*this, nullptr);
      for (auto& part : this->parts)
         part.clear(*this);
   }
   void BodyPartData::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->ragdoll.clear_if(*this, other);
      for (auto& part : this->parts)
         part.sever_outbound_references_to(*this, other);
   }
}