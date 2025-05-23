#include "./MagicEffect.h"
#include "./_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/magic_effect/invalid_actor_value_index.h"
#include "../notices/form_load_warnings/by_form_type/magic_effect/redundant_sound.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::magic_effect;
   }
}

namespace dovah::loaded_forms {
   void MagicEffect::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'CTDA':
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case 'MDOB':
               if (auto& form = this->menu_display_object; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::statik, subrecord.signature());
               }
               break;
            case 'DNAM':
               subrecord.read(this->description);
               break;
            case 'DATA':
               subrecord.read(this->flags);
               subrecord.read(this->base_cost);
               subrecord.read(this->associated_items.form);
               subrecord.read(this->magic_skill);
               {
                  auto& av = this->resist_av;
                  if (subrecord.read(av)) {
                     if (av < -2 && av >= all_actor_value_info.size()) {
                        specific_load_warnings::invalid_actor_value_index notice(
                           this->stub,
                           av,
                           specific_load_warnings::invalid_actor_value_index::which_type::resist
                        );
                        intfc.log_load_warning(notice);
                        //
                        av = -1;
                     }
                  }
               }
               subrecord.skip_bytes(2); // Counter Effect count
               subrecord.skip_bytes(2); // padding
               if (auto& form = this->vfx.casting.light; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::light, subrecord.signature());
               }
               subrecord.read(this->taper.weight);
               if (auto& form = this->vfx.hit.shader; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::effect_shader, subrecord.signature());
               }
               if (auto& form = this->vfx.enchant.shader; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::effect_shader, subrecord.signature());
               }
               subrecord.read(this->min_skill_level);
               subrecord.read(this->spellmaking.area);
               subrecord.read(this->spellmaking.casting_time);
               subrecord.read(this->taper.curve);
               subrecord.read(this->taper.duration);
               subrecord.read(this->associated_items.second_av_weight);
               subrecord.read(this->archetype);
               {
                  auto& av = this->associated_items.actor_value_indices[0];
                  if (subrecord.read(av)) {
                     if (av < -2 && av >= all_actor_value_info.size()) {
                        specific_load_warnings::invalid_actor_value_index notice(
                           this->stub,
                           av,
                           specific_load_warnings::invalid_actor_value_index::which_type::assoc_item_1
                        );
                        intfc.log_load_warning(notice);
                        //
                        av = -1;
                     }
                  }
               }
               if (auto& form = this->vfx.projectile; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::projectile, subrecord.signature());
               }
               if (auto& form = this->vfx.explosion; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::explosion, subrecord.signature());
               }
               subrecord.read(this->casting_type);
               subrecord.read(this->delivery_type);
               {
                  auto& av = this->associated_items.actor_value_indices[1];
                  if (subrecord.read(av)) {
                     if (av < -2 && av >= all_actor_value_info.size()) {
                        specific_load_warnings::invalid_actor_value_index notice(
                           this->stub,
                           av,
                           specific_load_warnings::invalid_actor_value_index::which_type::assoc_item_2
                        );
                        intfc.log_load_warning(notice);
                        //
                        av = -1;
                     }
                  }
               }
               if (auto& form = this->vfx.casting.art; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::art_object, subrecord.signature());
               }
               if (auto& form = this->vfx.hit.effect_art; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::art_object, subrecord.signature());
               }
               if (auto& form = this->vfx.impact_data_set; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::impact_data_set, subrecord.signature());
               }
               subrecord.read(this->skill_usage_mult);
               if (auto& form = this->dual_casting.dual_cast_data; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::dual_cast_data, subrecord.signature());
               }
               subrecord.read(this->dual_casting.scale);
               if (auto& form = this->vfx.enchant.art; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::art_object, subrecord.signature());
               }
               if (auto& form = this->vfx.unk_visual_effect_a; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::visual_effect, subrecord.signature());
               }
               if (auto& form = this->vfx.unk_visual_effect_b; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::visual_effect, subrecord.signature());
               }
               if (auto& form = this->equip_ability; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::spell, subrecord.signature());
               }
               if (auto& form = this->vfx.imagespace_modifier; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::imagespace_modifier, subrecord.signature());
               }
               if (auto& form = this->perk_to_apply; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::perk, subrecord.signature());
               }
               subrecord.read(this->audio.casting_loudness);
               subrecord.read(this->ai_params.score);
               subrecord.read(this->ai_params.cooldown);
               //
               // TESV.exe version upgrade branches. It's really weird that these are mutually exclusive... 
               // I can only assume that they exist as spot fixes for specific record versions, and that 
               // Bethesda was diligent about mass-updating MGEF records such that fixes never needed to 
               // be applied cumulatively. Otherwise, this code is just straight-up broken.
               //
               if (record.version() < 0x10) {
                  this->associated_items.second_av_weight = 0.0F;
                  this->taper.duration = 0.0F;
                  this->associated_items.actor_value_indices[1] = -1;
               } else if (record.version() < 0x14) {
                  this->skill_usage_mult = 1.0F;
               } else if (record.version() < 0x17) {
                  this->dual_casting.scale = 1.0F;
               } else if (record.version() < 0x19) {
                  this->vfx.enchant.art.unmanaged_set(nullptr);
               } else if (record.version() < 0x1A) {
                  this->equip_ability.unmanaged_set(nullptr);
               } else if (record.version() < 0x20) {
                  this->perk_to_apply.unmanaged_set(nullptr);
               }
               break;
            case 'SNDD':
               for (size_t i = 0; i + 7 < subrecord.size(); i += 8) {
                  size_t size = this->audio.sounds.size();
                  auto&  item = this->audio.sounds.emplace_back();
                  if (subrecord.read(item.type)) {
                     if (auto& form = item.descriptor; subrecord.read(form)) {
                        intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
                     }
                     //
                     // If a MGEF happens to specify multiple sounds of the same type, Skyrim only 
                     // retains the first one specified. We'll retain them all, but we should warn 
                     // about this situation.
                     //
                     for (size_t i = 0; i < size; ++i) {
                        if (this->audio.sounds[i].type == item.type) {
                           specific_load_warnings::redundant_sound notice(
                              this->stub,
                              item.type,
                              item.descriptor.get_form_stub()
                           );
                           intfc.log_load_warning(notice);
                           break;
                        }
                     }
                  }
               }
               break;
            case 'ESCE':
               {
                  form_reference_t form;
                  if (subrecord.read(form)) {
                     this->counter_effects.push_back(form);
                  }
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void MagicEffect::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t formID;
      form_id_t related;        // DATA+0x08
      form_id_t light;          // DATA+0x18
      form_id_t hit_shader;     // DATA+0x20
      form_id_t enchant_shader; // DATA+0x24
      form_id_t projectile;     // DATA+0x48
      form_id_t explosion;      // DATA+0x4C
      form_id_t casting_art;    // DATA+0x5C
      form_id_t hit_effect;     // DATA+0x60
      form_id_t impact_data;    // DATA+0x64
      form_id_t dual_cast;      // DATA+0x6C
      form_id_t enchant_art;    // DATA+0x74
      form_id_t equip_ability;  // DATA+0x80
      form_id_t imagespace_mod; // DATA+0x84
      form_id_t perk;           // DATA+0x88
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'MDOB': // menu display object
            case 'ESCE': // counter effects
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'DATA':
               subrecord.skip_bytes(8); // flags, base cost
               subrecord.read(related);
               subrecord.skip_bytes(12); // skill, resistance, unknown
               subrecord.read(light);
               subrecord.skip_bytes(4); // taper weight
               subrecord.read(hit_shader);
               subrecord.read(enchant_shader);
               subrecord.skip_bytes(32); // skill level, area, casting time, taper curve, taper duration, second AV weight, effect type, primary AV
               subrecord.read(projectile);
               subrecord.read(explosion);
               subrecord.skip_bytes(12);
               subrecord.read(casting_art);
               subrecord.read(hit_effect);
               subrecord.read(impact_data);
               subrecord.skip_bytes(4); // skill usage mult
               subrecord.read(dual_cast);
               subrecord.skip_bytes(4); // dual cast scale
               subrecord.read(enchant_art);
               subrecord.skip_bytes(8);
               subrecord.read(equip_ability);
               subrecord.read(imagespace_mod);
               subrecord.read(perk);
               subrecord.skip_bytes(12);
               if (record.version() < 0x10) {
                  ;
               } else if (record.version() < 0x14) {
                  ;
               } else if (record.version() < 0x17) {
                  ;
               } else if (record.version() < 0x19) {
                  enchant_art = {};
               } else if (record.version() < 0x1A) {
                  equip_ability = {};
               } else if (record.version() < 0x20) {
                  perk = {};
               }
               break;
            case 'SNDD':
               subrecord.skip_bytes(4);
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'DNAM': // description
               break;
         }
      }
      uib.add_outbound_reference(related);        // DATA+0x08
      uib.add_outbound_reference(light);          // DATA+0x18
      uib.add_outbound_reference(hit_shader);     // DATA+0x20
      uib.add_outbound_reference(enchant_shader); // DATA+0x24
      uib.add_outbound_reference(projectile);     // DATA+0x48
      uib.add_outbound_reference(explosion);      // DATA+0x4C
      uib.add_outbound_reference(casting_art);    // DATA+0x5C
      uib.add_outbound_reference(hit_effect);     // DATA+0x60
      uib.add_outbound_reference(impact_data);    // DATA+0x64
      uib.add_outbound_reference(dual_cast);      // DATA+0x6C
      uib.add_outbound_reference(enchant_art);    // DATA+0x74
      uib.add_outbound_reference(equip_ability);  // DATA+0x80
      uib.add_outbound_reference(imagespace_mod); // DATA+0x84
      uib.add_outbound_reference(perk);           // DATA+0x88
   }
   void MagicEffect::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (MagicEffect*)out;

      copy->keywords.clone_from(this->keywords, *copy);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);

      copy->name = this->name;
      copy->description = this->description;

      copy->flags = this->flags;
      copy->archetype = this->archetype;
      copy->casting_type = this->casting_type;
      copy->delivery_type = this->delivery_type;
      copy->magic_skill = this->magic_skill;
      copy->min_skill_level = this->min_skill_level;
      copy->base_cost = this->base_cost;
      copy->skill_usage_mult = this->skill_usage_mult;
      copy->resist_av = this->resist_av;
      copy->equip_ability.set(*copy, this->equip_ability);
      copy->perk_to_apply.set(*copy, this->perk_to_apply);
      copy->menu_display_object.set(*copy, this->menu_display_object);
      copy_form_reference_list(*copy, copy->counter_effects, this->counter_effects);

      copy->ai_params = this->ai_params;
      {
         auto& src = this->associated_items;
         auto& dst = copy->associated_items;
         dst.actor_value_indices = src.actor_value_indices;
         dst.form.set(*copy, src.form);
         dst.second_av_weight = src.second_av_weight;
      }
      {
         auto& src = this->audio;
         auto& dst = copy->audio;
         {
            const size_t size = src.sounds.size();
            for (auto& item : dst.sounds)
               item.descriptor.set(*copy, nullptr);
            dst.sounds.resize(size);
            for (size_t i = 0; i < size; ++i) {
               auto& src_item = src.sounds[i];
               auto& dst_item = dst.sounds[i];
               dst_item.type = src_item.type;
               dst_item.descriptor.set(*copy, src_item.descriptor);
            }
         }
         dst.casting_loudness = src.casting_loudness;
      }
      {
         auto& src = this->dual_casting;
         auto& dst = copy->dual_casting;
         dst.dual_cast_data.set(*copy, src.dual_cast_data);
         dst.scale = src.scale;
      }
      copy->spellmaking = this->spellmaking;
      copy->taper = this->taper;
      {
         auto& src = this->vfx;
         auto& dst = copy->vfx;
         dst.casting.art.set(*copy, src.casting.art);
         dst.casting.light.set(*copy, src.casting.light);
         dst.enchant.art.set(*copy, src.enchant.art);
         dst.enchant.shader.set(*copy, src.enchant.shader);
         dst.hit.effect_art.set(*copy, src.hit.effect_art);
         dst.hit.shader.set(*copy, src.hit.shader);
         dst.explosion.set(*copy, src.explosion);
         dst.imagespace_modifier.set(*copy, src.imagespace_modifier);
         dst.impact_data_set.set(*copy, src.impact_data_set);
         dst.projectile.set(*copy, src.projectile);
         dst.unk_visual_effect_a.set(*copy, src.unk_visual_effect_a);
         dst.unk_visual_effect_b.set(*copy, src.unk_visual_effect_b);
      }
   }
   void MagicEffect::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      if (!this->name.empty()) {
         auto& FULL = record.open_next_subrecord('FULL');
         FULL.write(this->name);
         FULL.close();
      }
      record.write_formID_subrecord('MDOB', this->menu_display_object, true);
      this->keywords.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->flags);
         subrecord.write(this->base_cost);
         subrecord.write(this->associated_items.form);
         subrecord.write(this->magic_skill);
         subrecord.write(this->resist_av);
         if (auto size = this->counter_effects.size(); size <= 0xFFFF) {
            subrecord.write((uint16_t)size);
         } else {
            subrecord.write((uint16_t)0xFFFF);
         }
         subrecord.skip_bytes(2); // padding
         subrecord.write(this->vfx.casting.light);
         subrecord.write(this->taper.weight);
         subrecord.write(this->vfx.hit.shader);
         subrecord.write(this->vfx.enchant.shader);
         subrecord.write(this->min_skill_level);
         subrecord.write(this->spellmaking.area);
         subrecord.write(this->spellmaking.casting_time);
         subrecord.write(this->taper.curve);
         subrecord.write(this->taper.duration);
         subrecord.write(this->associated_items.second_av_weight);
         subrecord.write(this->archetype);
         subrecord.write(this->associated_items.actor_value_indices[0]);
         subrecord.write(this->vfx.projectile);
         subrecord.write(this->vfx.explosion);
         subrecord.write(this->casting_type);
         subrecord.write(this->delivery_type);
         subrecord.write(this->associated_items.actor_value_indices[1]);
         subrecord.write(this->vfx.casting.art);
         subrecord.write(this->vfx.hit.effect_art);
         subrecord.write(this->vfx.impact_data_set);
         subrecord.write(this->skill_usage_mult);
         subrecord.write(this->dual_casting.dual_cast_data);
         subrecord.write(this->dual_casting.scale);
         subrecord.write(this->vfx.enchant.art);
         subrecord.write(this->vfx.unk_visual_effect_a);
         subrecord.write(this->vfx.unk_visual_effect_b);
         subrecord.write(this->equip_ability);
         subrecord.write(this->vfx.imagespace_modifier);
         subrecord.write(this->perk_to_apply);
         subrecord.write(this->audio.casting_loudness);
         subrecord.write(this->ai_params.score);
         subrecord.write(this->ai_params.cooldown);
         subrecord.close();
      }
      if (!this->counter_effects.empty()) {
         auto& subrecord = record.open_next_subrecord('ESCE');
         for (auto& item : this->counter_effects) {
            subrecord.write(item);
         }
         subrecord.close();
      }
      if (!this->audio.sounds.empty()) {
         auto& subrecord = record.open_next_subrecord('SNDD');
         for (auto& item : this->audio.sounds) {
            subrecord.write(item.type);
            subrecord.write(item.descriptor);
         }
         subrecord.close();
      }
      if (!this->description.empty()) {
         auto& DNAM = record.open_next_subrecord('DNAM');
         DNAM.write(this->description);
         DNAM.close();
      }
      for (auto& cnd : this->conditions) {
         cnd.save(record, intfc);
      }
   }
   void MagicEffect::_clear_impl() noexcept {
      this->conditions.clear(*this);
      this->keywords.clear(*this);
      this->script_data.clear(*this);

      this->name.reset();
      this->description.reset();

      this->flags = 0;
      this->archetype = magic_effect_archetype::value_modifier;
      this->casting_type = magic_casting_type::constant_effect;
      this->delivery_type = magic_delivery_type::self;
      this->base_cost = 0;
      this->magic_skill = -1;
      this->min_skill_level = 0;
      this->resist_av = -1;
      this->skill_usage_mult = 1.0F;

      clear_form_reference_list(this->counter_effects, *this);
      this->equip_ability.set(*this, nullptr);
      this->menu_display_object.set(*this, nullptr);
      this->perk_to_apply.set(*this, nullptr);

      this->ai_params = {};

      this->associated_items.form.set(*this, nullptr);
      this->associated_items.actor_value_indices = { -1, -1 };
      this->associated_items.second_av_weight = 0;

      for (auto& item : this->audio.sounds)
         item.descriptor.set(*this, nullptr);
      this->audio.sounds.clear();
      this->audio.casting_loudness = detection_loudness::normal;

      this->dual_casting.dual_cast_data.set(*this, nullptr);
      this->dual_casting.scale = 1.0F;

      this->spellmaking = {};
      this->taper = {};

      this->vfx.casting.art.set(*this, nullptr);
      this->vfx.casting.light.set(*this, nullptr);
      this->vfx.enchant.art.set(*this, nullptr);
      this->vfx.enchant.shader.set(*this, nullptr);
      this->vfx.hit.effect_art.set(*this, nullptr);
      this->vfx.hit.shader.set(*this, nullptr);
      this->vfx.explosion.set(*this, nullptr);
      this->vfx.imagespace_modifier.set(*this, nullptr);
      this->vfx.impact_data_set.set(*this, nullptr);
      this->vfx.projectile.set(*this, nullptr);
      this->vfx.unk_visual_effect_a.set(*this, nullptr);
      this->vfx.unk_visual_effect_b.set(*this, nullptr);
   }
   void MagicEffect::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->keywords.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);

      remove_form_from_reference_list(this->counter_effects, other, *this);
      this->equip_ability.clear_if(*this, other);
      this->menu_display_object.clear_if(*this, other);
      this->perk_to_apply.clear_if(*this, other);

      this->associated_items.form.clear_if(*this, other);

      for (auto& item : this->audio.sounds)
         item.descriptor.clear_if(*this, other);
      std::erase_if(this->audio.sounds, [](auto& item) { return item.descriptor == nullptr; });

      this->dual_casting.dual_cast_data.clear_if(*this, other);

      this->vfx.casting.art.clear_if(*this, other);
      this->vfx.casting.light.clear_if(*this, other);
      this->vfx.enchant.art.clear_if(*this, other);
      this->vfx.enchant.shader.clear_if(*this, other);
      this->vfx.hit.effect_art.clear_if(*this, other);
      this->vfx.hit.shader.clear_if(*this, other);
      this->vfx.explosion.clear_if(*this, other);
      this->vfx.imagespace_modifier.clear_if(*this, other);
      this->vfx.impact_data_set.clear_if(*this, other);
      this->vfx.projectile.clear_if(*this, other);
      this->vfx.unk_visual_effect_a.clear_if(*this, other);
      this->vfx.unk_visual_effect_b.clear_if(*this, other);
   }
}