#include "Weapon.h"
#include "_common_cpp.h"
#include "helpers/split_join_flags.h" // cobb::split_flags and cobb::join_flags

namespace dovah::loaded_forms {
   void Weapon::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'ICON':
               subrecord.read(this->item_data.icons.inventory);
               break;
            case 'MICO':
               subrecord.read(this->item_data.icons.message);
               break;
            case components::enchantable::subrecord_signature_effect:
            case components::enchantable::subrecord_signature_effect_legacy: // ENAM
            case components::enchantable::subrecord_signature_charge:
            case components::enchantable::subrecord_signature_charge_legacy: // ANAM
               this->enchantable.load(subrecord, intfc);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case 'ETYP':
               if (auto& dst = this->equip_type; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::equip_slot, subrecord.signature());
               break;
            case 'BIDS':
               if (auto& dst = this->block_bash.impact_data_set; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::impact_data_set, subrecord.signature());
               break;
            case 'BAMT':
               if (auto& dst = this->block_bash.alternate_material; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::material_type, subrecord.signature());
               break;
            case 'YNAM':
               if (auto& dst = this->item_data.sounds.take; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'ZNAM':
               if (auto& dst = this->item_data.sounds.drop; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'MOD3':
            case 'MO3S':
            case 'MO3T':
            case 'MO3D':
               this->scope_model.load(subrecord, intfc);
               break;
            case 'EFSD':
               subrecord.read(this->scope_shader);
               break;
            case 'NNAM':
               subrecord.read(this->embedded.node);
               break;
            case 'INAM':
               if (auto& dst = this->impact_data_set; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::impact_data_set, subrecord.signature());
               break;
            case 'WNAM':
               if (auto& dst = this->first_person_model; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::statik, subrecord.signature());
               break;
            case 'SNAM':
               if (auto& dst = this->sounds.attack; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'XNAM':
               if (auto& dst = this->sounds.attack_2D; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'NAM7':
               if (auto& dst = this->sounds.attack_loop; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'TNAM':
               if (auto& dst = this->sounds.attack_fail; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'UNAM':
               if (auto& dst = this->sounds.idle; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'NAM9':
               if (auto& dst = this->sounds.equip; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'NAM8':
               if (auto& dst = this->sounds.unequip; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'DATA':
               subrecord.read(this->item_data.value);
               subrecord.read(this->item_data.weight);
               subrecord.read(this->damage);
               break;
            case 'DNAM':
               subrecord.read(this->type);
               subrecord.skip_bytes(3);
               subrecord.read(this->speed);
               subrecord.read(this->reach);
               {
                  uint16_t flags = 0;
                  if (subrecord.read(flags)) {
                     cobb::split_flags<decltype(flags)>(
                        this->flags.ignores_normal_weapon_resist,
                        this->flags.automatic,
                        this->flags.has_scope,
                        this->flags.cant_drop,
                        this->flags.hide_backpack,
                        this->flags.embedded,
                        this->flags.no_first_person_ironsight_anim,
                        this->flags.non_playable
                     );
                  }
               }
               subrecord.skip_bytes(2);
               subrecord.read(this->ironsight_fov);
               subrecord.read(this->unk_dnam_10);
               subrecord.read(this->base_vats_hit_chance);
               subrecord.read(this->animation.legacy_anim);
               subrecord.read(this->projectile_count);
               subrecord.read(this->embedded.actor_value);
               subrecord.read(this->ai_ranges.minimum);
               subrecord.read(this->ai_ranges.maximum);
               subrecord.read(this->hit_gore_behavior);
               {
                  uint32_t flags = 0;
                  if (subrecord.read(flags)) {
                     cobb::split_flags<decltype(flags)>(
                        this->flags.player_only,
                        this->flags.npcs_use_ammo,
                        this->flags.never_jams_after_reload,
                        this->flags.unk_flag_b_3,
                        this->flags.minor_crime,
                        this->flags.fixed_ai_range,
                        this->flags.not_used_in_normal_combat,
                        this->flags.unk_flag_b_7,
                        this->flags.no_third_person_ironsight_anim,
                        this->flags.burst_shot,
                        this->flags.rumble_alternate,
                        this->flags.long_bursts,
                        this->flags.non_hostile,
                        this->flags.bound_weapon
                     );
                  }
               }
               subrecord.read(this->unk_dnam_30);
               subrecord.read(this->rumble.left_motor);
               subrecord.read(this->rumble.right_motor);
               subrecord.read(this->rumble.duration);
               subrecord.read(this->unk_dnam_40);
               subrecord.read(this->rumble.pattern);
               subrecord.skip_bytes(4); // DNAM+0x48 // unused
               subrecord.read(this->skill); // DNAM+0x4C
               subrecord.skip_bytes(8); // DNAM+0x50 // unused
               subrecord.read(this->resist_av); // DNAM+0x58
               subrecord.skip_bytes(4); // DNAM+0x5C // unused
               subrecord.read(this->stagger);
               break;
            case 'CRDT':
               subrecord.read(this->crit_data.added_damage);
               subrecord.skip_bytes(2);
               subrecord.read(this->crit_data.chance_mult);
               {
                  uint8_t flags = 0;
                  subrecord.read(flags);
                  this->crit_data.apply_spell_only_on_target_death = flags & 1;
               }
               if (subrecord.is_skyrim_special()) {
                  subrecord.skip_bytes(7);
               } else {
                  subrecord.skip_bytes(3);
               }
               if (auto& dst = this->crit_data.spell_to_apply; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::spell, subrecord.signature());
               if (subrecord.is_skyrim_special()) {
                  subrecord.skip_bytes(4);
               }
               break;
            case 'VNAM':
               subrecord.read(this->loudness);
               break;
            case 'CNAM':
               if (auto& dst = this->template_weapon; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::weapon, subrecord.signature());
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Weapon::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t equip_type;
      struct {
         form_id_t alternate_material;
         form_id_t impact_data_set;
      } block_bash;
      form_id_t crit_spell_to_apply;
      form_id_t impact_data_set;
      form_id_t first_person_model;
      struct {
         form_id_t attack;
         form_id_t attack_2D;
         form_id_t attack_loop;
         form_id_t attack_fail;
         form_id_t idle;
         form_id_t equip;
         form_id_t unequip;
      } sounds;
      form_id_t template_weapon;
      form_id_t take_sound;
      form_id_t drop_sound;
      form_id_t scope_effect;
      components::enchantable::use_info_state enchantable;
      components::destruction_stage_data::use_info_builder destruction_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
            case 'MOD3': // scope model
            case 'MO3S': // scope model
            case 'MO3T': // scope model
            case 'MO3D': // scope model
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'EFSD':
               subrecord.read(scope_effect);
               break;
            case components::enchantable::subrecord_signature_effect:
            case components::enchantable::subrecord_signature_effect_legacy: // ENAM
            case components::enchantable::subrecord_signature_charge:
            case components::enchantable::subrecord_signature_charge_legacy: // ANAM
               enchantable.read(subrecord);
               break;
            case 'DEST': // destruction stage header
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case 'ETYP':
               subrecord.read(equip_type);
               break;
            case 'BIDS':
               subrecord.read(block_bash.impact_data_set);
               break;
            case 'BAMT':
               subrecord.read(block_bash.alternate_material);
               break;
            case 'YNAM':
               subrecord.read(take_sound);
               break;
            case 'ZNAM':
               subrecord.read(drop_sound);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'INAM':
               subrecord.read(impact_data_set);
               break;
            case 'WNAM':
               subrecord.read(first_person_model);
               break;
            case 'SNAM':
               subrecord.read(sounds.attack);
               break;
            case 'XNAM':
               subrecord.read(sounds.attack_2D);
               break;
            case 'NAM7':
               subrecord.read(sounds.attack_loop);
               break;
            case 'TNAM':
               subrecord.read(sounds.attack_fail);
               break;
            case 'UNAM':
               subrecord.read(sounds.idle);
               break;
            case 'NAM9':
               subrecord.read(sounds.equip);
               break;
            case 'NAM8':
               subrecord.read(sounds.unequip);
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
            case 'DNAM':
               break;
            case 'CRDT':
               subrecord.skip_bytes(
                  2 + // added damage
                  2 + // padding
                  4 + // chance mult
                  1   // flags
               );
               if (subrecord.is_skyrim_special()) {
                  subrecord.skip_bytes(7);
               } else {
                  subrecord.skip_bytes(3);
               }
               subrecord.read(crit_spell_to_apply);
               break;
            case 'CNAM':
               subrecord.read(template_weapon);
               break;
         }
      }
      uib.add_outbound_reference(equip_type);
      uib.add_outbound_reference(block_bash.alternate_material);
      uib.add_outbound_reference(block_bash.impact_data_set);
      uib.add_outbound_reference(crit_spell_to_apply);
      uib.add_outbound_reference(impact_data_set);
      uib.add_outbound_reference(first_person_model);
      uib.add_outbound_reference(sounds.attack);
      uib.add_outbound_reference(sounds.attack_2D);
      uib.add_outbound_reference(sounds.attack_loop);
      uib.add_outbound_reference(sounds.attack_fail);
      uib.add_outbound_reference(sounds.idle);
      uib.add_outbound_reference(sounds.equip);
      uib.add_outbound_reference(sounds.unequip);
      uib.add_outbound_reference(template_weapon);
      uib.add_outbound_reference(take_sound);
      uib.add_outbound_reference(drop_sound);
      uib.add_outbound_reference(scope_effect);
      enchantable.commit(uib);
      destruction_uib.done();
   }
   void Weapon::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Weapon*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);
      copy->scope_model.clone_from(this->scope_model);
      copy->scope_shader.set(*copy, this->scope_shader);
      {
         auto& src_opt = this->destruction_data;
         auto& dst_opt = copy->destruction_data;
         if (dst_opt.has_value()) {
            dst_opt.value().clear(*copy);
            dst_opt = {};
         }
         if (src_opt.has_value()) {
            dst_opt.emplace();
            dst_opt.value().clone_from(src_opt.value(), *copy);
         }
      }
      copy->enchantable.clone_from(this->enchantable, *copy);
      copy->keywords.clone_from(this->keywords, *copy);

      copy->ai_ranges = this->ai_ranges;
      copy->animation = this->animation;
      copy->base_vats_hit_chance = this->base_vats_hit_chance;
      copy->description   = this->description;
      copy->flags         = this->flags;
      copy->hit_gore_behavior = this->hit_gore_behavior;
      copy->ironsight_fov = this->ironsight_fov;
      copy->loudness = this->loudness;
      copy->projectile_count = this->projectile_count;
      copy->type = this->type;
      copy->reach = this->reach;
      copy->resist_av = this->resist_av;
      copy->rumble = this->rumble;
      copy->skill = this->skill;
      copy->speed = this->speed;
      copy->stagger = this->stagger;
      copy->type  = this->type;
      copy->unk_dnam_10 = this->unk_dnam_10;
      copy->unk_dnam_30 = this->unk_dnam_30;
      copy->unk_dnam_40 = this->unk_dnam_40;

      copy->block_bash.alternate_material.set(*copy, this->block_bash.alternate_material);
      copy->block_bash.impact_data_set.set(*copy, this->block_bash.impact_data_set);

      copy->crit_data.added_damage = this->crit_data.added_damage;
      copy->crit_data.apply_spell_only_on_target_death = this->crit_data.apply_spell_only_on_target_death;
      copy->crit_data.chance_mult = this->crit_data.chance_mult;
      copy->crit_data.spell_to_apply.set(*copy, this->crit_data.spell_to_apply);

      copy->embedded = this->embedded;

      copy->item_data.icons  = this->item_data.icons;
      copy->item_data.value  = this->item_data.value;
      copy->item_data.weight = this->item_data.weight;
      copy->item_data.sounds.drop.set(*copy, this->item_data.sounds.drop);
      copy->item_data.sounds.take.set(*copy, this->item_data.sounds.take);

      copy->equip_type.set(*copy, this->equip_type);
      copy->first_person_model.set(*copy, this->first_person_model);
      copy->impact_data_set.set(*copy, this->impact_data_set);
      copy->sounds.attack.set(*copy, this->sounds.attack);
      copy->sounds.attack_2D.set(*copy, this->sounds.attack_2D);
      copy->sounds.attack_loop.set(*copy, this->sounds.attack_loop);
      copy->sounds.attack_fail.set(*copy, this->sounds.attack_fail);
      copy->sounds.idle.set(*copy, this->sounds.idle);
      copy->sounds.equip.set(*copy, this->sounds.equip);
      copy->sounds.unequip.set(*copy, this->sounds.unequip);
      copy->template_weapon.set(*copy, this->template_weapon);
   }
   void Weapon::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      {
         auto& subrecord = record.open_next_subrecord('FULL');
         subrecord.write(this->name);
         subrecord.close();
      }
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (auto& str = this->item_data.icons.inventory; !str.empty())
         record.write_string_subrecord('ICON', str);
      if (auto& str = this->item_data.icons.message; !str.empty())
         record.write_string_subrecord('MICO', str);
      this->enchantable.save(record, intfc);
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      record.write_formID_subrecord('ETYP', this->equip_type, true);
      record.write_formID_subrecord('BIDS', this->block_bash.impact_data_set, true);
      record.write_formID_subrecord('BAMT', this->block_bash.alternate_material, true);
      record.write_formID_subrecord('YNAM', this->item_data.sounds.take, true);
      record.write_formID_subrecord('ZNAM', this->item_data.sounds.drop, true);
      this->keywords.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DESC');
         subrecord.write(this->description);
         subrecord.close();
      }
      this->scope_model.save(record, intfc, 'MOD3', 'MO3T');
      if (auto& str = this->embedded.node; !str.empty())
         record.write_string_subrecord('NNAM', str);
      record.write_formID_subrecord('INAM', this->impact_data_set, true);
      record.write_formID_subrecord('WNAM', this->first_person_model, true);
      record.write_formID_subrecord('SNAM', this->sounds.attack, true);
      record.write_formID_subrecord('XNAM', this->sounds.attack_2D, true);
      record.write_formID_subrecord('NAM7', this->sounds.attack_loop, true);
      record.write_formID_subrecord('TNAM', this->sounds.attack_fail, true);
      record.write_formID_subrecord('UNAM', this->sounds.idle, true);
      record.write_formID_subrecord('NAM9', this->sounds.equip, true);
      record.write_formID_subrecord('NAM8', this->sounds.unequip, true);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->item_data.value);
         subrecord.write(this->item_data.weight);
         subrecord.write(this->damage);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->type);
         subrecord.skip_bytes(3);
         subrecord.write(this->speed);
         subrecord.write(this->reach);
         {
            auto flags = cobb::join_flags<uint16_t>(
               this->flags.ignores_normal_weapon_resist,
               this->flags.automatic,
               this->flags.has_scope,
               this->flags.cant_drop,
               this->flags.hide_backpack,
               this->flags.embedded,
               this->flags.no_first_person_ironsight_anim,
               this->flags.non_playable
            );
            subrecord.write(flags);
         }
         subrecord.skip_bytes(2);
         subrecord.write(this->ironsight_fov);
         subrecord.write(this->unk_dnam_10);
         subrecord.write(this->base_vats_hit_chance);
         subrecord.write(this->animation.legacy_anim);
         subrecord.write(this->projectile_count);
         subrecord.write(this->embedded.actor_value);
         subrecord.write(this->ai_ranges.minimum);
         subrecord.write(this->ai_ranges.maximum);
         subrecord.write(this->hit_gore_behavior);
         {
            auto flags = cobb::join_flags<uint32_t>(
               this->flags.player_only,
               this->flags.npcs_use_ammo,
               this->flags.never_jams_after_reload,
               this->flags.unk_flag_b_3,
               this->flags.minor_crime,
               this->flags.fixed_ai_range,
               this->flags.not_used_in_normal_combat,
               this->flags.unk_flag_b_7,
               this->flags.no_third_person_ironsight_anim,
               this->flags.burst_shot,
               this->flags.rumble_alternate,
               this->flags.long_bursts,
               this->flags.non_hostile,
               this->flags.bound_weapon
            );
            subrecord.write(flags);
         }
         subrecord.write(this->unk_dnam_30);
         subrecord.write(this->rumble.left_motor);
         subrecord.write(this->rumble.right_motor);
         subrecord.write(this->rumble.duration);
         subrecord.write(this->unk_dnam_40);
         subrecord.write(this->rumble.pattern);
         subrecord.skip_bytes(4); // DNAM+0x48 // unused
         subrecord.write(this->skill); // DNAM+0x4C
         subrecord.skip_bytes(8); // DNAM+0x50 // unused
         subrecord.write(this->resist_av); // DNAM+0x58
         subrecord.skip_bytes(4); // DNAM+0x5C // unused
         subrecord.write(this->stagger);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('CRDT');
         subrecord.write(this->crit_data.added_damage);
         subrecord.skip_bytes(2);
         subrecord.write(this->crit_data.chance_mult);
         {
            uint8_t flags = 0;
            if (this->crit_data.apply_spell_only_on_target_death)
               flags |= (1 << 0);
            subrecord.write(flags);
         }
         if (subrecord.is_skyrim_special()) {
            subrecord.skip_bytes(7);
         } else {
            subrecord.skip_bytes(3);
         }
         subrecord.write(this->crit_data.spell_to_apply);
         if (subrecord.is_skyrim_special()) {
            subrecord.skip_bytes(4);
         }
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('VNAM');
         subrecord.write(this->loudness);
         subrecord.close();
      }
      record.write_formID_subrecord('CNAM', this->template_weapon, true);
      record.write_formID_subrecord('EFSD', this->scope_shader, true);
   }
   void Weapon::_clear_impl() noexcept {
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->enchantable.clear(*this);
      this->keywords.clear(*this);
      this->model.clear(*this);
      this->scope_model.clear();
      this->script_data.clear(*this);
      //
      this->block_bash.alternate_material.set(*this, nullptr);
      this->block_bash.impact_data_set.set(*this, nullptr);
      this->crit_data.spell_to_apply.set(*this, nullptr);
      this->equip_type.set(*this, nullptr);
      this->first_person_model.set(*this, nullptr);
      this->impact_data_set.set(*this, nullptr);
      this->sounds.attack.set(*this, nullptr);
      this->sounds.attack_2D.set(*this, nullptr);
      this->sounds.attack_loop.set(*this, nullptr);
      this->sounds.attack_fail.set(*this, nullptr);
      this->sounds.idle.set(*this, nullptr);
      this->sounds.equip.set(*this, nullptr);
      this->sounds.unequip.set(*this, nullptr);
      this->template_weapon.set(*this, nullptr);
      this->item_data.sounds.take.set(*this, nullptr);
      this->item_data.sounds.drop.set(*this, nullptr);
      //
      this->name.reset();
      this->description.reset();
      this->crit_data.added_damage = 0;
      this->crit_data.apply_spell_only_on_target_death = false;
      this->crit_data.chance_mult = 1.0F;
      this->embedded = {};
      this->item_data.icons = {};
      this->item_data.value = 0;
      this->item_data.weight = 0;
      //
      this->type = weapon_type::one_hand_dagger;
      this->speed = 0;
      this->reach = 0;
      this->flags = {};
      this->ironsight_fov = 0;
      this->unk_dnam_10 = 0;
      this->unk_dnam_30 = 0;
      this->unk_dnam_40 = 0;
      this->base_vats_hit_chance = 0;
      this->animation = {};
      this->projectile_count = 1;
      this->ai_ranges = {};
      this->hit_gore_behavior = {};
      this->rumble = {};
      this->skill = skill::one_handed;
      this->resist_av = -1;
      this->stagger = 0;
      this->loudness = detection_loudness::normal;
   }
   void Weapon::_sever_outbound_references_impl(form_stub& other) noexcept {
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->enchantable.sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->scope_model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      //
      this->block_bash.alternate_material.set(*this, nullptr);
      this->block_bash.impact_data_set.set(*this, nullptr);
      this->crit_data.spell_to_apply.set(*this, nullptr);
      this->equip_type.set(*this, nullptr);
      this->first_person_model.set(*this, nullptr);
      this->impact_data_set.set(*this, nullptr);
      this->sounds.attack.set(*this, nullptr);
      this->sounds.attack_2D.set(*this, nullptr);
      this->sounds.attack_loop.set(*this, nullptr);
      this->sounds.attack_fail.set(*this, nullptr);
      this->sounds.idle.set(*this, nullptr);
      this->sounds.equip.set(*this, nullptr);
      this->sounds.unequip.set(*this, nullptr);
      this->template_weapon.set(*this, nullptr);
      this->item_data.sounds.take.set(*this, nullptr);
      this->item_data.sounds.drop.set(*this, nullptr);
   }
}