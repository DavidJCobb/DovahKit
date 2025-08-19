#include "./FormSubdialogPerkEntry.h"
#include "dovah/data/actor_values.h"
#include "dovah/forms/Perk.h"
#include "editor/localize/entry_point_function.h"
#include "editor/localize/perk_entry_point.h"
#include "ui/utils/set_range.h"
#include "widgets/DKConditionList.h"

namespace {
   enum class entry_type {
      quest,
      spell,
      entry_point,
   };
}

FormSubdialogPerkEntry::FormSubdialogPerkEntry(dovah::loaded_forms::Perk& perk, QWidget* parent) : QDialog(parent), _state({ .form = perk }) {
   this->ui.setupUi(this);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   {
      auto* widget = this->ui.type;
      widget->clear();
      widget->addItem(tr("Quest"), (int)entry_type::quest);
      widget->addItem(tr("Ability"), (int)entry_type::spell);
      widget->addItem(tr("Entry Point"), (int)entry_type::entry_point);
   }
   
   this->ui.questForm->setAllowedFormType(dovah::form_type::quest);
   ui::set_range<uint16_t>(this->ui.questStage);

   this->ui.abilityForm->setAllowedFormType(dovah::form_type::spell);

   {
      auto* widget = this->ui.entryPointParamsOneAVOneFloat_AV;
      widget->clear();
      for (auto& av_info : dovah::all_actor_value_info) {
         widget->addItem(av_info.name.data(), (int)av_info.index);
      }
   }
   this->ui.entryPointParamsActivateChoice_Spell->setAllowedFormType(dovah::form_type::spell);

   QObject::connect(this->ui.entryPointType, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
      auto* widget = this->ui.entryPointType;
      auto  entry  = (dovah::perk_entry_point)widget->currentData().toInt();
      if ((size_t)entry >= dovah::all_perk_entry_points.size()) {
         this->_rebuild_entry_point_condition_tabs(nullptr);
         return;
      }
      const auto& info = dovah::all_perk_entry_points[(size_t)entry];
      this->_rebuild_entry_point_function_type_combobox(info.value_type);
      this->_rebuild_entry_point_condition_tabs(&info);
   });
   QObject::connect(this->ui.entryPointFunction, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
      this->_update_entry_point_arguments((dovah::entry_point_function)this->ui.entryPointFunction->currentData().toInt());
   });
   
   //
   // Now that the event handlers are in place, set up the entry point combobox. This'll trigger signals that 
   // call the various other "update" functions.
   //
   {
      using value_type = dovah::perk_entry_point;
      constexpr const auto mapping = std::array{
         #pragma region [0, 10)
            std::pair{ value_type::calc_weapon_damage, "Calc Weapon Damage" },
            std::pair{ value_type::calc_my_crit_chance, "Calc My Crit Chance" },
            std::pair{ value_type::calc_my_crit_damage, "Calc My Crit Damage" },
            std::pair{ value_type::calc_mine_explode_chance, "Calc Mine Explode Chance" },
            std::pair{ value_type::adjust_limb_damage, "Adjust Limb Damage" },
            std::pair{ value_type::adjust_book_skill_points, "Adjust Book Skill Points" },
            std::pair{ value_type::mod_recovered_health, "Mod Recovered Health" },
            std::pair{ value_type::get_should_attack, "Get Should Attack" },
            std::pair{ value_type::mod_buy_prices, "Mod Buy Prices" },
            std::pair{ value_type::add_leveled_item_on_death, "Add Leveled Item on Death" },
         #pragma endregion
         #pragma region [10, 20)
            std::pair{ value_type::get_max_carry_weight, "Get Max Carry Weight" },
            std::pair{ value_type::mod_addiction_chance, "Mod Addiction Chance" },
            std::pair{ value_type::mod_addiction_duration, "Mod Addiction Duration" },
            std::pair{ value_type::mod_positive_chem_duration, "Mod Positive Chem Duration" },
            std::pair{ value_type::activate, "Activate" },
            std::pair{ value_type::ignore_running_during_detection, "Ignore Running During Detection" },
            std::pair{ value_type::ignore_broken_lock, "Ignore Broken Lock" },
            std::pair{ value_type::mod_enemy_crit_chance, "Mod Enemy Crit Chance" },
            std::pair{ value_type::mod_sneak_attack_mult, "Mod Sneak Attack Mult" },
            std::pair{ value_type::mod_max_placeable_mines, "Mod Max Placeable Mines" },
         #pragma endregion
         #pragma region [20, 30)
            std::pair{ value_type::mod_bow_zoom, "Mod Bow Zoom" },
            std::pair{ value_type::mod_recover_arrow_chance, "Mod Recover Arrow Chance" },
            std::pair{ value_type::mod_skill_use, "Mod Skill Use" },
            std::pair{ value_type::mod_telekinesis_distance, "Mod Telekinesis Distance" },
            std::pair{ value_type::mod_telekinesis_damage_mult, "Mod Telekinesis Damage Mult" },
            std::pair{ value_type::mod_telekinesis_damage, "Mod Telekinesis Damage" },
            std::pair{ value_type::mod_bash_damage, "Mod Bash Damage" },
            std::pair{ value_type::mod_power_attack_cost, "Mod Power Attack Cost" },
            std::pair{ value_type::mod_power_attack_damage, "Mod Power Attack Damage" },
            std::pair{ value_type::mod_outgoing_spell_magnitude, "Mod Outgoing Spell Magnitude" },
         #pragma endregion
         #pragma region [30, 40)
            std::pair{ value_type::mod_outgoing_spell_duration, "Mod Outgoing Spell Duration" },
            std::pair{ value_type::mod_magic_second_av_weight, "Mod Magic Second AV Weight" },
            std::pair{ value_type::mod_armor_weight, "Mod Armor Weight" },
            std::pair{ value_type::mod_incoming_stagger, "Mod Incoming Stagger" },
            std::pair{ value_type::mod_outgoing_stagger, "Mod Outgoing Stagger" },
            std::pair{ value_type::mod_attack_damage, "Mod Attack Damage" },
            std::pair{ value_type::mod_incoming_damage, "Mod Incoming Damage" },
            std::pair{ value_type::mod_target_damage_resist, "Mod Target Damage Resist" },
            std::pair{ value_type::mod_spell_cost, "Mod Spell Cost" },
            std::pair{ value_type::mod_percent_blocked, "Mod Percent Blocked" },
         #pragma endregion
         #pragma region [40, 50)
            std::pair{ value_type::mod_shield_deflect_arrow_chance, "Mod Shield Deflect Arrow Chance" },
            std::pair{ value_type::mod_incoming_spell_magnitude, "Mod Incoming Spell Magnitude" },
            std::pair{ value_type::mod_incoming_spell_duration, "Mod Incoming Spell Duration" },
            std::pair{ value_type::mod_player_intimidation, "Mod Player Intimidation" },
            std::pair{ value_type::mod_player_reputation, "Mod Player Reputation" },
            std::pair{ value_type::mod_favor_points, "Mod Favor Points" },
            std::pair{ value_type::mod_bribe_amount, "Mod Bribe Amount" },
            std::pair{ value_type::mod_detection_light, "Mod Detection Light" },
            std::pair{ value_type::mod_detection_movement, "Mod Detection Movement" },
            std::pair{ value_type::mod_soul_gem_recharge, "Mod Soul Gem Recharge" },
         #pragma endregion
         #pragma region [50, 60)
            std::pair{ value_type::set_sweep_attack, "Set Sweep Attack" },
            std::pair{ value_type::apply_combat_hit_spell, "Apply Combat Hit Spell" },
            std::pair{ value_type::apply_bash_spell, "Apply Bash Spell" },
            std::pair{ value_type::apply_reanimate_spell, "Apply Reanimate Spell" },
            std::pair{ value_type::set_boolean_graph_variable, "Set Boolean Animation Graph Variable" },
            std::pair{ value_type::mod_spell_casting_sound_event, "Mod Spell Casting Sound Event" },
            std::pair{ value_type::mod_pickpocket_chance, "Mod Pickpocket Chance" },
            std::pair{ value_type::mod_detection_sneak_skill, "Mod Detection Sneak Skill" },
            std::pair{ value_type::mod_fall_damage, "Mod Fall Damage" },
            std::pair{ value_type::mod_lockpick_sweet_spot, "Mod Lockpick Sweet Spot" },
         #pragma endregion
         #pragma region [60, 70)
            std::pair{ value_type::mod_sell_prices, "Mod Sell Prices" },
            std::pair{ value_type::can_pickpocket_equipped_item, "Can Pickpocket Equipped Items" },
            std::pair{ value_type::mod_lockpick_level_allowed, "Mod Lockpick Level Allowed" },
            std::pair{ value_type::set_lockpick_starting_arc, "Set Lockpick Starting Arc" },
            std::pair{ value_type::set_progression_picking, "Set Progression Picking" },
            std::pair{ value_type::set_lockpicks_unbreakable, "Set Lockpicks Unbreakable" },
            std::pair{ value_type::mod_alchemy_effectiveness, "Mod Alchemy Effectiveness" },
            std::pair{ value_type::apply_weapon_swing_spell, "Apply Weapon Swing Spell" },
            std::pair{ value_type::mod_commanded_actor_limit, "Mod Commanded Actor Limit" },
            std::pair{ value_type::apply_sneak_spell, "Apply Sneak Spell" },
         #pragma endregion
         #pragma region [70, 80)
            std::pair{ value_type::mod_player_magic_slowdown, "Mod Player Magic Slowdown" },
            std::pair{ value_type::mod_ward_magicka_absorb_percent, "Mod Ward Magicka Absorb %" },
            std::pair{ value_type::mod_initial_ingredient_effects_learned, "Mod Initial Ingredient Effects Learned" },
            std::pair{ value_type::purify_alchemy_ingredients, "Purify Alchemy Ingredients" },
            std::pair{ value_type::filter_activation, "Filter Activation" },
            std::pair{ value_type::can_dual_cast_spell, "Can Dual-Cast Spell" },
            std::pair{ value_type::mod_tempering_health, "Mod Tempering Health" },
            std::pair{ value_type::mod_enchantment_power, "Mod Enchantment Power" },
            std::pair{ value_type::mod_soul_percent_captured_to_weapon, "Mod Soul % Captured to Weapon" },
            std::pair{ value_type::mod_soul_gem_enchanting, "Mod Soul Gem Enchanting" },
         #pragma endregion
         #pragma region [80, 90)
            std::pair{ value_type::mod_num_enchantments_allowed, "Mod Num Enchantments Allowed" },
            std::pair{ value_type::set_activate_label, "Set Activate Label" },
            std::pair{ value_type::mod_shout_okay, "Mod Shout Okay" },
            std::pair{ value_type::mod_poison_dose_count, "Mod Poison Dose Count" },
            std::pair{ value_type::should_apply_placed_item, "Should Apply Placed Item" },
            std::pair{ value_type::mod_armor_rating, "Mod Armor Rating" },
            std::pair{ value_type::mod_lockpicking_crime_chance, "Mod Lockpicking Crime Chance" },
            std::pair{ value_type::mod_ingredients_harvested, "Mod Ingredients Harvested" },
            std::pair{ value_type::mod_spell_range, "Mod Spell Range" },
            std::pair{ value_type::mod_alchemy_potions_created, "Mod Alchemy Potions Created" },
         #pragma endregion
         #pragma region [90, 100)
            std::pair{ value_type::mod_lockpicking_key_reward_chance, "Mod Lockpicking Key Reward Chance" },
            std::pair{ value_type::allow_mount_actor, "Allow Mount Actor" },
         #pragma endregion
      };

      auto* widget = this->ui.entryPointType;
      widget->clear();
      for (auto& info : dovah::all_perk_entry_points) {
         auto name = editor::localize::perk_entry_point(info.id);
         widget->addItem(name, (int)info.id);
      }
   }

   this->_update_options();
   QObject::connect(this->ui.type, qOverload<int>(&QComboBox::currentIndexChanged), this, &FormSubdialogPerkEntry::_update_options);
}


FormSubdialogPerkEntry::value_type FormSubdialogPerkEntry::value() const {
   value_type dst;
   dst.priority = this->ui.priority->value();
   dst.rank     = this->ui.rank->value();
   switch ((entry_type)this->ui.type->currentIndex()) {
      case entry_type::quest:
         {
            auto& dst_casted = dst.data.emplace<ui::types::perk_entries::quest_entry>();
            dst_casted.quest = this->ui.questForm->formStub();
            dst_casted.stage = this->ui.questStage->value();
         }
         break;
      case entry_type::spell:
         {
            auto& dst_casted = dst.data.emplace<ui::types::perk_entries::spell_entry>();
            dst_casted.spell = this->ui.abilityForm->formStub();
         }
         break;
      case entry_type::entry_point:
         {
            auto& dst_casted = dst.data.emplace<ui::types::perk_entries::entry_point_entry>();
            dst_casted.entry_point = (dovah::perk_entry_point)this->ui.entryPointType->currentData().toInt();
            dst_casted.set_function((dovah::entry_point_function)this->ui.entryPointFunction->currentData().toInt());
            switch (dovah::entry_point_value_type_of(dst_casted.function)) {
               case dovah::entry_point_value_type::activate_choice:
                  {
                     auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::activate_choice>();
                     params.label = this->ui.entryPointParamsActivateChoice_Text->text();
                     params.spell = this->ui.entryPointParamsActivateChoice_Spell->formStub();
                     params.replace_default = this->ui.entryPointParamsActivateChoice_ReplaceDefault->isChecked();
                     params.run_immediately = this->ui.entryPointParamsActivateChoice_RunImmediately->isChecked();
                     {
                        auto* src = this->ui.entryPointParamsActivateChoice_Fragment;
                        auto& dst = params.fragment;
                        dst.script   = src->currentScriptname();
                        dst.function = src->currentFunction();
                     }
                  }
                  break;
               case dovah::entry_point_value_type::leveled_item:
               case dovah::entry_point_value_type::spell:
                  {
                     auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::form>();
                     params.value = this->ui.entryPointParamsForm_Form->formStub();
                  }
                  break;
               case dovah::entry_point_value_type::localized_string:
                  {
                     auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::localized_string>();
                     params.value = this->ui.entryPointParamsLocalizedString_Text->text();
                  }
                  break;
               case dovah::entry_point_value_type::none:
                  dst_casted.parameters.emplace<std::monostate>();
                  break;
               case dovah::entry_point_value_type::number:
                  {
                     auto* container = this->ui.entryPointParamsStack->currentWidget();
                     if (container == this->ui.entryPointParamsOneAVOneFloat) {
                        auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::one_av_one_float>();
                        params.actor_value = this->ui.entryPointParamsOneAVOneFloat_AV->currentData().toInt();
                        params.value       = this->ui.entryPointParamsOneAVOneFloat_Float->value();
                     } else if (container == this->ui.entryPointParamsOneFloat) {
                        auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::one_float>();
                        params.value = this->ui.entryPointParamsOneFloat_Value->value();
                     } else if (container == this->ui.entryPointParamsTwoFloats) {
                        auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::two_floats>();
                        params.values[0] = this->ui.entryPointParamsTwoFloats_ValueA->value();
                        params.values[1] = this->ui.entryPointParamsTwoFloats_ValueB->value();
                     }
                  }
                  break;
               case dovah::entry_point_value_type::raw_string:
                  {
                     auto& params = dst_casted.parameters.emplace<ui::types::perk_entries::params::raw_string>();
                     params.value = this->ui.entryPointParamsRawString_Text->text();
                  }
                  break;
            }

            const size_t size = this->ui.entryPointConditionsTabbox->count();
            dst_casted.conditions_by_entity.resize(size);
            for (size_t i = 0; i < size; ++i) {
               auto* widget = qobject_cast<DKConditionList*>(this->ui.entryPointConditionsTabbox->widget(i));
               if (!widget)
                  break;
               widget->exportTo(this->_state.form, dst_casted.conditions_by_entity[i].conditions);
            }
         }
         break;
   }
   return dst;
}
void FormSubdialogPerkEntry::setValue(const value_type& src) {
   this->ui.rank->setValue(src.rank);
   this->ui.priority->setValue(src.priority);
   if (auto* casted = std::get_if<ui::types::perk_entries::quest_entry>(&src.data)) {
      this->ui.type->setCurrentIndex(this->ui.type->findData((int)entry_type::quest));
      this->ui.questForm->setFormStub(casted->quest);
      this->ui.questStage->setValue(casted->stage);
   } else if (auto* casted = std::get_if<ui::types::perk_entries::spell_entry>(&src.data)) {
      this->ui.type->setCurrentIndex(this->ui.type->findData((int)entry_type::spell));
      this->ui.abilityForm->setFormStub(casted->spell);
   } else if (auto* casted = std::get_if<ui::types::perk_entries::entry_point_entry>(&src.data)) {
      this->ui.type->setCurrentIndex(this->ui.type->findData((int)entry_type::entry_point));
      {
         auto* widget = this->ui.entryPointType;
         auto  i      = widget->findData((int)casted->entry_point);
         widget->setCurrentIndex(i);
      }
      {
         auto* widget = this->ui.entryPointFunction;
         auto  i      = widget->findData((int)casted->function);
         widget->setCurrentIndex(i);
      }
      if (auto* params = std::get_if<ui::types::perk_entries::params::activate_choice>(&casted->parameters)) {
         this->ui.entryPointParamsActivateChoice_Text->setText(params->label);
         this->ui.entryPointParamsActivateChoice_Spell->setFormStub(params->spell);
         this->ui.entryPointParamsActivateChoice_ReplaceDefault->setChecked(params->replace_default);
         this->ui.entryPointParamsActivateChoice_RunImmediately->setChecked(params->run_immediately);
         {
            auto* widget = this->ui.entryPointParamsActivateChoice_Fragment;
            widget->setCurrentScriptname(params->fragment.script);
            widget->setCurrentFunction(params->fragment.function);
         }
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::form>(&casted->parameters)) {

         this->ui.entryPointParamsForm_Form->setFormStub(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::localized_string>(&casted->parameters)) {
         this->ui.entryPointParamsLocalizedString_Text->setText(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::one_av_one_float>(&casted->parameters)) {
         {
            auto* widget = this->ui.entryPointParamsOneAVOneFloat_AV;
            widget->setCurrentIndex(widget->findData(params->actor_value));
         }
         this->ui.entryPointParamsOneAVOneFloat_Float->setValue(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::one_float>(&casted->parameters)) {
         this->ui.entryPointParamsOneFloat_Value->setValue(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::raw_string>(&casted->parameters)) {
         this->ui.entryPointParamsRawString_Text->setText(params->value);
      } else if (auto* params = std::get_if<ui::types::perk_entries::params::two_floats>(&casted->parameters)) {
         this->ui.entryPointParamsTwoFloats_ValueA->setValue(params->values[0]);
         this->ui.entryPointParamsTwoFloats_ValueB->setValue(params->values[1]);
      }

      const size_t size = casted->conditions_by_entity.size();
      for (size_t i = 0; i < size; ++i) {
         auto* widget = qobject_cast<DKConditionList*>(this->ui.entryPointConditionsTabbox->widget(i));
         if (!widget)
            break;
         widget->importFrom(this->_state.form, casted->conditions_by_entity[i].conditions);
      }
   }
}

void FormSubdialogPerkEntry::_update_options() {
   switch ((entry_type)this->ui.type->currentIndex()) {
      case entry_type::quest:
         this->ui.optionsStack->setCurrentWidget(this->ui.optionsQuest);
         break;
      case entry_type::spell:
         this->ui.optionsStack->setCurrentWidget(this->ui.optionsAbility);
         break;
      case entry_type::entry_point:
         this->ui.optionsStack->setCurrentWidget(this->ui.optionsEntryPoint);
         break;
   }
}
void FormSubdialogPerkEntry::_rebuild_entry_point_condition_tabs(const dovah::perk_entry_point_info* info) {
   auto* tabview = this->ui.entryPointConditionsTabbox;
   size_t size = tabview->count();
   for (size_t i = 0; i < size; ++i) {
      auto* tab = tabview->widget(0);
      tabview->removeTab(0);
      tab->deleteLater();
   }
   size = 0;
   if (info) {
      size = info->arg_count();
      for (size_t i = 0; i < size; ++i) {
         auto& subject = info->args[i];
         auto* tab     = new DKConditionList(this);
         tabview->addTab(tab, QString(subject.name));
      }
   }
   if (!size) {
      auto* tab = new DKConditionList(this);
      tab->setEnabled(false);
      tabview->addTab(tab, "Perk Owner");
      tabview->setTabEnabled(0, false);
   }
}
void FormSubdialogPerkEntry::_rebuild_entry_point_function_type_combobox(dovah::entry_point_value_type vt) {
   using value_type = dovah::entry_point_function;
   constexpr const auto list = std::array{
      value_type::none,
      value_type::set_value,
      value_type::add_value,
      value_type::multiply_value,
      value_type::add_range_to_value,
      value_type::add_actor_value_mult,
      value_type::absolute_value,
      value_type::negative_absolute_value,
      value_type::add_leveled_list,
      value_type::add_activate_choice,
      value_type::select_spell,
      value_type::select_text,
      value_type::set_to_actor_value_mult,
      value_type::multiply_actor_value_mult,
      value_type::multiply_one_plus_av_mult,
      value_type::set_text,
   };

   auto*      widget  = this->ui.entryPointFunction;
   auto       prior   = (value_type)widget->currentData().toInt();
   const auto blocker = QSignalBlocker(widget);
   widget->clear();
   for (auto v : list) {
      auto pair_type = dovah::entry_point_value_type_of(v);
      if (pair_type != vt)
         continue;
      auto name = editor::localize::entry_point_function(v);
      widget->addItem(name, (int)v);
   }
   auto i = widget->findData((int)prior);
   if (i >= 0)
      widget->setCurrentIndex(i);

   auto after = (value_type)widget->currentData().toInt();
   this->_update_entry_point_arguments(after);
}
void FormSubdialogPerkEntry::_update_entry_point_arguments(dovah::entry_point_function func) {
   auto  type  = dovah::expected_type_for_entry_point_function(func);
   auto* stack = this->ui.entryPointParamsStack;
   switch (type) {
      case dovah::entry_point_function_type::activate_choice:
         stack->setCurrentWidget(this->ui.entryPointParamsActivateChoice);
         break;
      case dovah::entry_point_function_type::animation_graph_var:
         this->ui.entryPointParamsRawString_Label->setText(tr("Variable Name:"));
         stack->setCurrentWidget(this->ui.entryPointParamsRawString);
         break;
      case dovah::entry_point_function_type::leveled_item:
         this->ui.entryPointParamsForm_Desc->setText(tr("Leveled Item:"));
         this->ui.entryPointParamsForm_Form->setAllowedFormType(dovah::form_type::leveled_item);
         stack->setCurrentWidget(this->ui.entryPointParamsForm);
         break;
      case dovah::entry_point_function_type::localized_string:
         stack->setCurrentWidget(this->ui.entryPointParamsLocalizedString);
         break;
      case dovah::entry_point_function_type::spell:
         this->ui.entryPointParamsForm_Desc->setText(tr("Spell:"));
         this->ui.entryPointParamsForm_Form->setAllowedFormType(dovah::form_type::spell);
         stack->setCurrentWidget(this->ui.entryPointParamsForm);
         break;
      case dovah::entry_point_function_type::one_float:
         stack->setCurrentWidget(this->ui.entryPointParamsOneFloat);
         {
            auto* label = this->ui.entryPointParamsOneFloat_Prefix;
            switch (func) {
               case dovah::entry_point_function::add_value:
                  label->setText(tr("Value = Value + "));
                  break;
               case dovah::entry_point_function::multiply_value:
                  label->setText(tr("Value = Value * "));
                  break;
               case dovah::entry_point_function::set_value:
                  label->setText(tr("Value = "));
                  break;
            }
         }
         break;
      case dovah::entry_point_function_type::two_floats:
         if (dovah::entry_point_function_takes_an_av(func)) {
            stack->setCurrentWidget(this->ui.entryPointParamsOneAVOneFloat);
            auto* label_a = this->ui.entryPointParamsOneAVOneFloat_Prefix;
            auto* label_b = this->ui.entryPointParamsOneAVOneFloat_Middle;
            auto* label_c = this->ui.entryPointParamsOneAVOneFloat_Suffix;
            switch (func) {
               case dovah::entry_point_function::add_actor_value_mult:
                  label_a->setText(tr("Value = Value + ("));
                  label_b->setText(tr(" * "));
                  label_c->setText(tr(")"));
                  break;
               case dovah::entry_point_function::multiply_actor_value_mult:
                  label_a->setText(tr("Value = Value * "));
                  label_b->setText(tr(" * "));
                  label_c->setText(tr(""));
                  break;
               case dovah::entry_point_function::multiply_one_plus_av_mult:
                  label_a->setText(tr("Value = Value * (1 + ("));
                  label_b->setText(tr(" * "));
                  label_c->setText(tr("))"));
                  break;
               case dovah::entry_point_function::set_to_actor_value_mult:
                  label_a->setText(tr("Value = "));
                  label_b->setText(tr(" * "));
                  label_c->setText(tr(""));
                  break;
            }
         } else {
            stack->setCurrentWidget(this->ui.entryPointParamsTwoFloats);
            auto* label_a = this->ui.entryPointParamsTwoFloats_Prefix;
            auto* label_b = this->ui.entryPointParamsTwoFloats_Middle;
            auto* label_c = this->ui.entryPointParamsTwoFloats_Suffix;
            switch (func) {
               case dovah::entry_point_function::add_range_to_value:
                  label_a->setText(tr("Value = Value + rand("));
                  label_b->setText(tr(", "));
                  label_c->setText(tr(")"));
                  break;
            }
         }
         break;
      case dovah::entry_point_function_type::none:
         stack->setCurrentWidget(this->ui.entryPointParamsNone);
         {
            auto* label = this->ui.entryPointParamsNone_Desc;
            switch (func) {
               case dovah::entry_point_function::none:
                  label->setText(tr(""));
                  break;
               case dovah::entry_point_function::absolute_value:
                  label->setText(tr("Value = abs(Value)"));
                  break;
               case dovah::entry_point_function::negative_absolute_value:
                  label->setText(tr("Value = -abs(Value)"));
                  break;
            }
         }
         break;
   }

}