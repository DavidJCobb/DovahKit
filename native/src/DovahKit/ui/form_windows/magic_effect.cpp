#include "./magic_effect.h"
#include <limits>
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/enum_dropdown_configs/detection_loudness.h"
#include "ui/utils/enum_dropdown_configs/magic_school_av_index.h"
#include "ui/utils/enum_dropdown_configs/resistance_av_index.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "./magic_effect/MagicEffectSummonableActorPickerFilter.h"
#include "./shared/DKFormPickerExcludeSingleFormFilter.h"

FormDialogMagicEffect::FormDialogMagicEffect(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->_filters.exclude_self = new DKFormPickerExcludeSingleFormFilter(this);

   this->_filters.summonable_actors = new MagicEffectSummonableActorPickerFilter(this);

   {
      auto* widget = this->ui.archetype;
      widget->clear();
      widget->addItem(tr("Absorb"), (int)dovah::magic_effect_archetype::absorb);
      widget->addItem(tr("Accumulate Magnitude"), (int)dovah::magic_effect_archetype::accumulate_magnitude);
      widget->addItem(tr("Banish"), (int)dovah::magic_effect_archetype::banish);
      widget->addItem(tr("Bound Weapon"), (int)dovah::magic_effect_archetype::bound_weapon);
      widget->addItem(tr("Calm"), (int)dovah::magic_effect_archetype::calm);
      widget->addItem(tr("Cloak"), (int)dovah::magic_effect_archetype::cloak);
      widget->addItem(tr("Command Summoned"), (int)dovah::magic_effect_archetype::command_summoned);
      widget->addItem(tr("Concussion"), (int)dovah::magic_effect_archetype::concussion);
      widget->addItem(tr("Cure Addiction"), (int)dovah::magic_effect_archetype::cure_addiction);
      widget->addItem(tr("Cure Disease"), (int)dovah::magic_effect_archetype::cure_disease);
      widget->addItem(tr("Cure Paralysis"), (int)dovah::magic_effect_archetype::cure_paralysis);
      widget->addItem(tr("Cure Poison"), (int)dovah::magic_effect_archetype::cure_poison);
      widget->addItem(tr("Darkness (Unused)"), (int)dovah::magic_effect_archetype::darkness);
      widget->addItem(tr("Demoralize"), (int)dovah::magic_effect_archetype::demoralize);
      widget->addItem(tr("Detect Life"), (int)dovah::magic_effect_archetype::detect_life);
      widget->addItem(tr("Disarm"), (int)dovah::magic_effect_archetype::disarm);
      widget->addItem(tr("Disguise"), (int)dovah::magic_effect_archetype::disguise);
      widget->addItem(tr("Dispel"), (int)dovah::magic_effect_archetype::dispel);
      widget->addItem(tr("Dual Value Modifier"), (int)dovah::magic_effect_archetype::dual_value_modifier);
      widget->addItem(tr("Enhance Weapon"), (int)dovah::magic_effect_archetype::enhance_weapon);
      widget->addItem(tr("Etherealize"), (int)dovah::magic_effect_archetype::etherealize);
      widget->addItem(tr("Frenzy"), (int)dovah::magic_effect_archetype::frenzy);
      widget->addItem(tr("Grab Actor"), (int)dovah::magic_effect_archetype::grab_actor);
      widget->addItem(tr("Guide"), (int)dovah::magic_effect_archetype::guide);
      widget->addItem(tr("Invisibility"), (int)dovah::magic_effect_archetype::invisibility);
      widget->addItem(tr("Light"), (int)dovah::magic_effect_archetype::light);
      widget->addItem(tr("Lock"), (int)dovah::magic_effect_archetype::lock);
      widget->addItem(tr("Night Eye (Unused)"), (int)dovah::magic_effect_archetype::night_eye);
      widget->addItem(tr("Open"), (int)dovah::magic_effect_archetype::open);
      widget->addItem(tr("Paralysis"), (int)dovah::magic_effect_archetype::paralysis);
      widget->addItem(tr("Peak Value Modifier"), (int)dovah::magic_effect_archetype::peak_value_modifier);
      widget->addItem(tr("Rally"), (int)dovah::magic_effect_archetype::rally);
      widget->addItem(tr("Reanimate"), (int)dovah::magic_effect_archetype::reanimate);
      widget->addItem(tr("Script"), (int)dovah::magic_effect_archetype::script);
      widget->addItem(tr("Slow Time"), (int)dovah::magic_effect_archetype::slow_time);
      widget->addItem(tr("Soul Trap"), (int)dovah::magic_effect_archetype::soul_trap);
      widget->addItem(tr("Spawn Hazard"), (int)dovah::magic_effect_archetype::spawn_hazard);
      widget->addItem(tr("Spawn Scripted Ref"), (int)dovah::magic_effect_archetype::spawn_scripted_ref);
      widget->addItem(tr("Stagger"), (int)dovah::magic_effect_archetype::stagger);
      widget->addItem(tr("Summon Creature"), (int)dovah::magic_effect_archetype::summon_creature);
      widget->addItem(tr("Telekinesis"), (int)dovah::magic_effect_archetype::telekinesis);
      widget->addItem(tr("Turn Undead"), (int)dovah::magic_effect_archetype::turn_undead);
      widget->addItem(tr("Value and Parts"), (int)dovah::magic_effect_archetype::value_and_parts);
      widget->addItem(tr("Value Modifier"), (int)dovah::magic_effect_archetype::value_modifier);
      widget->addItem(tr("Vampire Lord"), (int)dovah::magic_effect_archetype::vampire_lord);
      widget->addItem(tr("Werewolf"), (int)dovah::magic_effect_archetype::werewolf);
      widget->addItem(tr("Werewolf Feed"), (int)dovah::magic_effect_archetype::werewolf_feed);
      widget->model()->sort(0);
   }
   {
      auto* widget = this->ui.castingType;
      widget->clear();
      widget->addItem(tr("Concentration"), (int)dovah::magic_casting_type::concentration);
      widget->addItem(tr("Constant Effect"), (int)dovah::magic_casting_type::constant_effect);
      widget->addItem(tr("Fire and Forget"), (int)dovah::magic_casting_type::fire_and_forget);
      //widget->addItem(tr("Scroll"), (int)dovah::magic_casting_type::scroll);
   }
   {
      auto* widget = this->ui.delivery;
      widget->clear();
      widget->addItem(tr("Aimed"), (int)dovah::magic_delivery_type::aimed);
      widget->addItem(tr("Self"), (int)dovah::magic_delivery_type::self);
      widget->addItem(tr("Target Actor"), (int)dovah::magic_delivery_type::target_actor);
      widget->addItem(tr("Target Location"), (int)dovah::magic_delivery_type::target_location);
      widget->addItem(tr("Touch"), (int)dovah::magic_delivery_type::touch);

      this->ui.flagSnapToNavmesh->setEnabled(false);
      QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, widget]() {
         auto delivery = (dovah::magic_delivery_type) widget->currentData().toInt();
         this->ui.flagSnapToNavmesh->setEnabled(delivery == dovah::magic_delivery_type::target_location);
      });
   }
   ui::enum_dropdown_configs::magic_school_av_index(this->ui.skill);
   this->ui.minSkill->setRange(0, 100);
   this->ui.assocItem1->setAllowedFormTypes({ dovah::form_type::file_header }); // Deliberately using an impossible form type here, to force the picker to empty.
   this->ui.assocItem2->setAllowedFormTypes({ dovah::form_type::file_header }); // Deliberately using an impossible form type here, to force the picker to empty.
   ui::set_range<float>(this->ui.secondAVWeight);
   ui::enum_dropdown_configs::resistance_av_index(this->ui.resistAV);
   this->ui.perkToApply->setAllowedFormType(dovah::form_type::perk);
   ui::set_unsigned_range<float>(this->ui.taperDuration);
   ui::set_range<float>(this->ui.taperWeight);
   ui::set_range<float>(this->ui.taperCurve);
   ui::set_unsigned_range<float>(this->ui.baseCost);
   ui::set_unsigned_range<float>(this->ui.skillUsageMult);
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });
   this->ui.counterEffects->setAllowedFormTypes({ dovah::form_type::magic_effect });
   this->ui.counterEffects->setCustomFilter(this->_filters.exclude_self);

   this->ui.menuDisplayObject->setAllowedFormType(dovah::form_type::statik);
   this->ui.castingArt->setAllowedFormType(dovah::form_type::art_object);
   this->ui.castingLight->setAllowedFormType(dovah::form_type::light);
   this->ui.enchantArt->setAllowedFormType(dovah::form_type::art_object);
   this->ui.enchantShader->setAllowedFormType(dovah::form_type::effect_shader);
   this->ui.hitEffectArt->setAllowedFormType(dovah::form_type::art_object);
   this->ui.hitShader->setAllowedFormType(dovah::form_type::effect_shader);
   this->ui.projectile->setAllowedFormType(dovah::form_type::projectile);
   this->ui.impactDataSet->setAllowedFormType(dovah::form_type::impact_data_set);
   this->ui.explosion->setAllowedFormType(dovah::form_type::explosion);
   this->ui.imagespaceModifier->setAllowedFormType(dovah::form_type::imagespace_modifier);
   this->ui.dualCastData->setAllowedFormType(dovah::form_type::dual_cast_data);
   ui::set_unsigned_range<float>(this->ui.dualCastScale);
   ui::set_unsigned_range<int32_t>(this->ui.spellmakingArea);
   ui::set_unsigned_range<float>(this->ui.spellmakingCastingTime);
   ui::set_unsigned_range<float>(this->ui.aiScore);
   ui::set_unsigned_range<float>(this->ui.aiDelayTime);
   this->ui.equipAbility->setAllowedFormType(dovah::form_type::spell);

   this->ui.soundDrawSheathe->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundCastLoop->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundCharge->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundReady->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundRelease->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundCastLoop->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundOnHit->setAllowedFormType(dovah::form_type::sound_descriptor);
   ui::enum_dropdown_configs::detection_loudness(this->ui.detectionSoundLevel);

   this->load(); // this creates the working copy.
}
void FormDialogMagicEffect::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   this->_filters.exclude_self->set_exclusion(this->formStub());
   
   #pragma region Left column
      ui::bind(this->ui.editorID, this->editor_id());
      this->ui.name->setText(gls.convert_localized_string(working.name));
      ui::bind(this->ui.archetype, working.archetype);
      QObject::connect(this->ui.archetype, qOverload<int>(&QComboBox::currentIndexChanged), this, &FormDialogMagicEffect::on_archetype_changed);
      ui::bind(this->ui.castingType, working.casting_type);
      ui::bind(this->ui.delivery, working.delivery_type);
      {
         auto* widget = this->ui.skill;
         auto& target = working.magic_skill;
         {
            auto i = widget->findData((int)target);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
         QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [widget, &target](int i) {
            if (i < 0) {
               target = -1;
            } else {
               target = widget->currentData().toInt();
            }
         });
      }
      ui::bind(this->ui.minSkill, working.min_skill_level);
      QObject::connect(this->ui.assocItem1, &DKFormPicker::formChanged, this, &FormDialogMagicEffect::on_associated_items_changed);
      QObject::connect(this->ui.assocItem2, &DKFormPicker::formChanged, this, &FormDialogMagicEffect::on_associated_items_changed);
      this->on_archetype_changed(true);
      ui::bind(this->ui.secondAVWeight, working.associated_items.second_av_weight);
      {
         auto* widget = this->ui.resistAV;
         auto& target = working.resist_av;
         {
            auto i = widget->findData((int)target);
            if (i >= 0)
               widget->setCurrentIndex(i);
         }
         QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [widget, &target](int i) {
            if (i < 0) {
               target = -1;
            } else {
               target = widget->currentData().toInt();
            }
         });
      }
      ui::bind(this->ui.perkToApply, working.perk_to_apply, working);
      ui::bind(this->ui.taperDuration, working.taper.duration);
      ui::bind(this->ui.taperWeight, working.taper.weight);
      ui::bind(this->ui.taperCurve, working.taper.curve);
      ui::bind(this->ui.baseCost, working.base_cost);
      ui::bind(this->ui.skillUsageMult, working.skill_usage_mult);
      this->ui.keywords->pullStubs(working.keywords.forms);
      this->ui.counterEffects->pullStubs(working.counter_effects);
   #pragma endregion

   this->ui.conditions->importFrom(working, working.conditions);
   {
      using flag = loaded_form_type::effect_flag;
      ui::bind(this->ui.flagDetrimental, working.flags, flag::detrimental);
      ui::bind(this->ui.flagFXPersist, working.flags, flag::fx_persist);
      ui::bind(this->ui.flagGory, working.flags, flag::gory_visuals);
      ui::bind(this->ui.flagHideInUI, working.flags, flag::hide_in_ui);
      ui::bind(this->ui.flagHostile, working.flags, flag::hostile);
      ui::bind(this->ui.flagNoArea, working.flags, flag::no_area);
      ui::bind(this->ui.flagNoDeathDispel, working.flags, flag::no_death_dispel);
      ui::bind(this->ui.flagNoDuration, working.flags, flag::no_duration);
      ui::bind(this->ui.flagNoHitEffect, working.flags, flag::no_hit_effect);
      ui::bind(this->ui.flagNoHitEvent, working.flags, flag::no_hit_event);
      ui::bind(this->ui.flagNoMagnitude, working.flags, flag::no_magnitude);
      ui::bind(this->ui.flagNoRecast, working.flags, flag::no_recast);
      ui::bind(this->ui.flagPainless, working.flags, flag::painless);
      ui::bind(this->ui.flagRecover, working.flags, flag::recover);
      ui::bind(this->ui.flagSnapToNavmesh, working.flags, flag::snap_to_navmesh);

      ui::bind(this->ui.dispelOthersWithKeyword, working.flags, flag::dispel_with_keywords);

      ui::bind(this->ui.flagPowerAffectsDuration, working.flags, flag::power_affects_duration);
      ui::bind(this->ui.flagPowerAffectsMagnitude, working.flags, flag::power_affects_magnitude);
   }

   #pragma region Middle column
      ui::bind(this->ui.menuDisplayObject, working.menu_display_object, working);
      ui::bind(this->ui.castingArt, working.vfx.casting.art, working);
      ui::bind(this->ui.castingLight, working.vfx.casting.light, working);
      ui::bind(this->ui.hitEffectArt, working.vfx.hit.effect_art, working);
      ui::bind(this->ui.hitShader, working.vfx.hit.shader, working);
      ui::bind(this->ui.enchantArt, working.vfx.enchant.art, working);
      ui::bind(this->ui.enchantShader, working.vfx.enchant.shader, working);
      ui::bind(this->ui.projectile, working.vfx.projectile, working);
      ui::bind(this->ui.impactDataSet, working.vfx.impact_data_set, working);
      ui::bind(this->ui.explosion, working.vfx.explosion, working);
      ui::bind(this->ui.imagespaceModifier, working.vfx.imagespace_modifier, working);

      ui::bind(this->ui.dualCastData, working.dual_casting.dual_cast_data, working);
      ui::bind(this->ui.dualCastScale, working.dual_casting.scale);

      ui::bind(this->ui.spellmakingArea, working.spellmaking.area);
      ui::bind(this->ui.spellmakingCastingTime, working.spellmaking.casting_time);

      ui::bind(this->ui.aiScore, working.ai_params.score);
      ui::bind(this->ui.aiDelayTime, working.ai_params.cooldown);

      ui::bind(this->ui.equipAbility, working.equip_ability, working);
   #pragma endregion

   #pragma region Right column
      {  // Sounds
         using effect_sound_type = loaded_form_type::effect_sound_type;

         auto _bind = [&working](DKFormPicker* picker, dovah::form_stub*& live_value, effect_sound_type type) {
            live_value = nullptr;
            for (auto& item : working.audio.sounds) {
               if (item.type == type) {
                  live_value = item.descriptor.get_form_stub();
                  break;
               }
            }
            ui::bind(picker, live_value);
         };
         _bind(this->ui.soundDrawSheathe, this->_state.sounds.draw_sheathe, effect_sound_type::draw_sheathe);
         _bind(this->ui.soundCharge, this->_state.sounds.charge, effect_sound_type::charge);
         _bind(this->ui.soundReady, this->_state.sounds.ready, effect_sound_type::ready);
         _bind(this->ui.soundRelease, this->_state.sounds.release, effect_sound_type::release);
         _bind(this->ui.soundCastLoop, this->_state.sounds.concentration_cast_loop, effect_sound_type::concentration_cast_loop);
         _bind(this->ui.soundOnHit, this->_state.sounds.on_hit, effect_sound_type::on_hit);
      }
      ui::bind(this->ui.detectionSoundLevel, working.audio.casting_loudness);

      this->ui.description->setPlainText(gls.convert_localized_string(working.description));

      this->ui.scriptListPane->setFormWorkingCopy(&working);
   #pragma endregion
}
void FormDialogMagicEffect::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.name, this->ui.name->text());
   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.counterEffects->commitStubs(working.counter_effects, working);

   this->ui.conditions->exportTo(working, working.conditions);

   {
      using effect_sound_type = loaded_form_type::effect_sound_type;

      auto& src = this->_state.sounds;
      auto& dst = working.audio.sounds;
      for (auto& item : dst)
         item.descriptor.set(working, nullptr);
      dst.clear();

      #pragma push_macro("DO_SOUND")
      #define DO_SOUND(name) \
         if (auto* stub = (src.name)) {          \
            auto& item = dst.emplace_back();     \
            item.type = effect_sound_type::name; \
            item.descriptor.set(working, stub);  \
         }
      DO_SOUND(draw_sheathe);
      DO_SOUND(charge);
      DO_SOUND(ready);
      DO_SOUND(release);
      DO_SOUND(concentration_cast_loop);
      DO_SOUND(on_hit);
      #pragma pop_macro("DO_SOUND")
   }
   //
   gls.assign_localized_string(working.description, this->ui.description->toPlainText());
   this->ui.scriptListPane->commit();
}

constexpr const auto no_associated_item = dovah::magic_effect_archetype_info::associated_item_type(dovah::form_type::none);

std::array<FormDialogMagicEffect::associated_item_constraint, 2> FormDialogMagicEffect::constraints_for_archetype() const {
   std::array<FormDialogMagicEffect::associated_item_constraint, 2> out = {};
   static_assert(out.size() <= std::tuple_size_v<decltype(dovah::magic_effect_archetype_info::associated_items)>);

   int archetype_index = this->ui.archetype->currentData().toInt();
   if (archetype_index < 0 || archetype_index >= dovah::all_magic_effect_archetypes.size()) {
      return out;
   }

   const auto& archetype_info = dovah::all_magic_effect_archetypes[archetype_index];
   auto& editor = DovahKitCore::get();
   for (size_t i = 0; i < out.size(); ++i) {
      const auto& src = archetype_info.associated_items[i];
      auto& dst = out[i];
      if (std::holds_alternative<dovah::form_type>(src)) {
         dst.form_type = std::get<dovah::form_type>(src);
         if (dst.form_type == dovah::form_type::actor_base) {
            if (archetype_info.flags.actor_base_must_be_summonable) {
               dst.actors_must_be_summonable = true;
            }
         }
      } else if (std::holds_alternative<const dovah::actor_value_info*>(src)) {
         auto* av_info = std::get<const dovah::actor_value_info*>(src);
         if (av_info) {
            auto* av_stub = editor.get_form_of_probable_type(dovah::form_type::actor_value_info, av_info->formID);
            if (av_stub) {
               dst.form_type = dovah::form_type::actor_value_info;
               dst.forced_av = av_stub;
            }
         }
      }
   }
   return out;
}
void FormDialogMagicEffect::on_archetype_changed(bool initial_load) {
   auto& working = *this->form;

   this->ui.assocItem1->setCustomFilter(nullptr);
   this->ui.assocItem2->setCustomFilter(nullptr);

   int archetype_index = this->ui.archetype->currentData().toInt();
   if (archetype_index < 0 || archetype_index >= dovah::all_magic_effect_archetypes.size()) {
      this->ui.assocItem1->setEnabled(false);
      this->ui.assocItem2->setEnabled(false);
      //
      // We want to force the formpickers to empty in situations like this, so we set their allowed 
      // form type to an impossible one. (We can't use dovah::form_type::none because some hardcoded 
      // forms, like the PapyrusPersistenceForm, actually have NONE as their type.)
      //
      this->ui.assocItem1->setAllowedFormType(dovah::form_type::file_header);
      this->ui.assocItem2->setAllowedFormType(dovah::form_type::file_header);
      return;
   }

   auto& editor = DovahKitCore::get();
   const auto& archetype_info = dovah::all_magic_effect_archetypes[archetype_index];
   const auto  widgets = std::array{
      this->ui.assocItem1,
      this->ui.assocItem2,
   };
   static_assert(widgets.size() <= std::tuple_size_v<decltype(dovah::magic_effect_archetype_info::associated_items)>);

   {
      auto& item = archetype_info.associated_items[1];
      if (std::holds_alternative<dovah::form_type>(item) && std::get<dovah::form_type>(item) == dovah::form_type::actor_value_info) {
         this->ui.secondAVWeight->setEnabled(true);
      } else {
         this->ui.secondAVWeight->setEnabled(false);
      }
   }

   const auto item_constraints = this->constraints_for_archetype();
   for (size_t i = 0; i < widgets.size(); ++i) {
      auto* widget  = widgets[i];
      auto  blocker = QSignalBlocker(widget);

      auto& ic = item_constraints[i];
      if (ic.form_type == dovah::form_type::none) {
         widget->setEnabled(false);
         widget->setAllowedFormType(dovah::form_type::file_header); // See comment above.
         continue;
      }
      widget->setAllowedFormType(ic.form_type);
      if (ic.form_type == dovah::form_type::actor_value_info) {
         if (ic.forced_av) {
            widget->setEnabled(false);
            widget->setFormStub(ic.forced_av);
         } else {
            widget->setEnabled(true);
            if (initial_load) {
               dovah::form_stub* av_stub = nullptr;
               {
                  auto av_index = working.associated_items.actor_value_indices[i];
                  if (av_index > 0 && av_index < dovah::all_actor_value_info.size()) {
                     const auto& av_info = dovah::all_actor_value_info[av_index];
                     av_stub = editor.get_form_of_probable_type(dovah::form_type::actor_value_info, av_info.formID);
                  }
               }
               widget->setFormStub(av_stub);
            }
         }
      } else {
         widget->setEnabled(true);
         if (ic.actors_must_be_summonable) {
            widget->setCustomFilter(this->_filters.summonable_actors);
         }
         if (initial_load) {
            widget->setFormStub(working.associated_items.form.get_form_stub());
         }
      }
   }
   this->on_associated_items_changed();
}
void FormDialogMagicEffect::on_associated_items_changed() {
   auto& working = *this->form;

   int archetype_index = this->ui.archetype->currentData().toInt();
   if (archetype_index < 0 || archetype_index >= dovah::all_magic_effect_archetypes.size())
      return;

   const auto item_constraints = this->constraints_for_archetype();
   const auto widgets = std::array{
      this->ui.assocItem1,
      this->ui.assocItem2,
   };

   dovah::form_stub* associated_form = nullptr;
   for (size_t i = 0; i < widgets.size(); ++i) {
      auto& constraint = item_constraints[i];
      auto* widget     = widgets[i];
      working.associated_items.actor_value_indices[i] = -1;
      if (constraint.form_type == dovah::form_type::none) {
         continue;
      }
      auto* stub = widget->formStub();
      if (constraint.form_type != dovah::form_type::actor_value_info) {
         associated_form = stub;
         continue;
      }
      if (constraint.forced_av) {
         stub = constraint.forced_av;
      }
      if (stub) {
         for (const auto& av_info : dovah::all_actor_value_info) {
            if (av_info.formID == stub->formID) {
               working.associated_items.actor_value_indices[i] = av_info.index;
               break;
            }
         }
      }
   }
   working.associated_items.form.set(working, associated_form);
}