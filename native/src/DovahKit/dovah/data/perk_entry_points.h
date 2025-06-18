#pragma once
#include <array>
#include "../form_types.h"
#include "./entry_point_functions.h"

namespace dovah {
   enum class perk_entry_point {
      calc_weapon_damage,
      calc_my_crit_chance,
      calc_my_crit_damage,
      calc_mine_explode_chance,
      adjust_limb_damage,
      adjust_book_skill_points,
      mod_recovered_health,
      get_should_attack,
      mod_buy_prices,
      add_leveled_item_on_death,
      get_max_carry_weight = 10,
      mod_addiction_chance,
      mod_addiction_duration,
      mod_positive_chem_duration,
      activate,
      ignore_running_during_detection = 15,
      ignore_broken_lock,
      mod_enemy_crit_chance,
      mod_sneak_attack_mult,
      mod_max_placeable_mines,
      mod_bow_zoom,
      mod_recover_arrow_chance,
      mod_skill_use,
      mod_telekinesis_distance,
      mod_telekinesis_damage_mult,
      mod_telekinesis_damage,
      mod_bash_damage,
      mod_power_attack_cost,
      mod_power_attack_damage,
      mod_outgoing_spell_magnitude,
      mod_outgoing_spell_duration,
      mod_magic_second_av_weight, // TODO: verify
      mod_armor_weight,
      mod_incoming_stagger,
      mod_outgoing_stagger,
      mod_attack_damage,
      mod_incoming_damage,
      mod_target_damage_resist,
      mod_spell_cost,
      mod_percent_blocked,
      mod_shield_deflect_arrow_chance,
      mod_incoming_spell_magnitude,
      mod_incoming_spell_duration,
      mod_player_intimidation,
      mod_player_reputation,
      mod_favor_points,
      mod_bribe_amount,
      mod_detection_light,
      mod_detection_movement,
      mod_soul_gem_recharge,
      set_sweep_attack,
      apply_combat_hit_spell,
      apply_bash_spell,
      apply_reanimate_spell,
      set_boolean_graph_variable,
      mod_spell_casting_sound_event,
      mod_pickpocket_chance,
      mod_detection_sneak_skill,
      mod_fall_damage,
      mod_lockpick_sweet_spot,
      mod_sell_prices,
      can_pickpocket_equipped_item,
      mod_lockpick_level_allowed,
      set_lockpick_starting_arc,
      set_progression_picking,
      set_lockpicks_unbreakable,
      mod_alchemy_effectiveness,
      apply_weapon_swing_spell,
      mod_commanded_actor_limit,
      apply_sneak_spell,
      mod_player_magic_slowdown,
      mod_ward_magicka_absorb_percent,
      mod_initial_ingredient_effects_learned,
      purify_alchemy_ingredients,
      filter_activation,
      can_dual_cast_spell,
      mod_tempering_health,
      mod_enchantment_power,
      mod_soul_percent_captured_to_weapon,
      mod_soul_gem_enchanting,
      mod_num_enchantments_allowed,
      set_activate_label,
      mod_shout_okay,
      mod_poison_dose_count,
      should_apply_placed_item,
      mod_armor_rating,
      mod_lockpicking_crime_chance,
      mod_ingredients_harvested,
      mod_spell_range, // Target Location
      mod_alchemy_potions_created,
      mod_lockpicking_key_reward_chance, // 90
      allow_mount_actor, // 91
   };

   struct perk_entry_point_info {
      public:
         enum class value_type {
            number,
            string,
         };

         struct condition_subject {
            const char*      name = "";
            dovah::form_type type = dovah::form_type::none;
         };

      public:
         // Convenience constants for the "all perk entry points" list.
         static constexpr const auto perk_owner_condition_subject = condition_subject{
            .name = "Perk Owner",
            .type = dovah::form_type::actor,
         };

         struct condition_subjects { // poor man's namespace
            condition_subjects() = delete;
            
            static constexpr const std::array<condition_subject, 3> for_incoming_attack = {
               perk_owner_condition_subject,
               condition_subject{
                  .name = "Attacker",
                  .type = dovah::form_type::actor,
               },
               condition_subject{
                  .name = "Attacker Weapon",
                  .type = dovah::form_type::weapon,
               },
            };
            static constexpr const std::array<condition_subject, 3> for_outgoing_attack = {
               perk_owner_condition_subject,
               condition_subject{
                  .name = "Weapon",
                  .type = dovah::form_type::weapon,
               },
               condition_subject{
                  .name = "Target",
                  .type = dovah::form_type::reference,
               },
            };
         };

      public:
         perk_entry_point id;
         std::array<condition_subject, 3> args = { perk_owner_condition_subject };
         entry_point_function_type function_type = entry_point_function_type::none;
   };

   inline constexpr const auto all_perk_entry_points = std::array{
      perk_entry_point_info{
         .id   = perk_entry_point::calc_weapon_damage,
         .args = perk_entry_point_info::condition_subjects::for_outgoing_attack,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::calc_my_crit_chance,
         .args = perk_entry_point_info::condition_subjects::for_outgoing_attack,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::calc_my_crit_damage,
         .args = perk_entry_point_info::condition_subjects::for_outgoing_attack,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::calc_mine_explode_chance,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::reference, // TODO: Verify
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::adjust_limb_damage,
         .args = perk_entry_point_info::condition_subjects::for_incoming_attack,
      },
      perk_entry_point_info{
         .id = perk_entry_point::adjust_book_skill_points,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_recovered_health,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::get_should_attack,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Attacker",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_buy_prices,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::add_leveled_item_on_death,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
         .function_type = entry_point_function_type::leveled_item,
      },
      perk_entry_point_info{
         .id = perk_entry_point::get_max_carry_weight,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_addiction_chance,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_addiction_duration,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_positive_chem_duration,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::activate,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
         .function_type = entry_point_function_type::two_floats,
      },
      perk_entry_point_info{
         .id = perk_entry_point::ignore_running_during_detection,
      },
      perk_entry_point_info{
         .id = perk_entry_point::ignore_broken_lock,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_enemy_crit_chance,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Weapon",
               .type = dovah::form_type::weapon,
            },
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_sneak_attack_mult,
         .args = perk_entry_point_info::condition_subjects::for_outgoing_attack,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_max_placeable_mines,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_bow_zoom,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Weapon",
               .type = dovah::form_type::weapon,
            },
         },
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_recover_arrow_chance,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_skill_use,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_telekinesis_distance,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_telekinesis_damage_mult,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_telekinesis_damage,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_bash_damage,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_power_attack_cost,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Weapon",
               .type = dovah::form_type::weapon,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_power_attack_damage,
         .args = perk_entry_point_info::condition_subjects::for_outgoing_attack,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_outgoing_spell_magnitude,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_outgoing_spell_duration,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_magic_second_av_weight,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_armor_weight,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::armor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_incoming_stagger,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Attacker",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_outgoing_stagger,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_attack_damage,
         .args = perk_entry_point_info::condition_subjects::for_outgoing_attack,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_incoming_damage,
         .args = perk_entry_point_info::condition_subjects::for_incoming_attack,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_target_damage_resist,
         .args = perk_entry_point_info::condition_subjects::for_outgoing_attack,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_spell_cost,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_percent_blocked,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_shield_deflect_arrow_chance,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_incoming_spell_magnitude,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_incoming_spell_duration,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_player_intimidation,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_player_reputation,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_favor_points,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_bribe_amount,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_detection_light,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_detection_movement,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_soul_gem_recharge,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::soul_gem, // TODO: verify
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::set_sweep_attack,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Weapon",
               .type = dovah::form_type::weapon, // TODO: verify
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::apply_combat_hit_spell,
         .args = perk_entry_point_info::condition_subjects::for_outgoing_attack,
         .function_type = entry_point_function_type::spell,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::apply_bash_spell,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
         .function_type = entry_point_function_type::spell,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::apply_reanimate_spell,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{ // the spell used to perform reanimation?
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
         .function_type = entry_point_function_type::spell,
      },
      perk_entry_point_info{
         .id = perk_entry_point::set_boolean_graph_variable,
         .function_type = entry_point_function_type::animation_graph_var,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_spell_casting_sound_event,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_pickpocket_chance,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::misc_item, // TODO: change this
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_detection_sneak_skill,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_fall_damage,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_lockpick_sweet_spot,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Locked Ref",
               .type = dovah::form_type::reference,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_sell_prices,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::can_pickpocket_equipped_item,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::misc_item, // TODO: change this
            },
         },
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_lockpick_level_allowed,
      },
      perk_entry_point_info{
         .id = perk_entry_point::set_lockpick_starting_arc,
      },
      perk_entry_point_info{
         .id = perk_entry_point::set_progression_picking,
      },
      perk_entry_point_info{
         .id = perk_entry_point::set_lockpicks_unbreakable,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_alchemy_effectiveness,
      },
      perk_entry_point_info{
         .id = perk_entry_point::apply_weapon_swing_spell,
         .args = perk_entry_point_info::condition_subjects::for_incoming_attack,
         .function_type = entry_point_function_type::spell,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_commanded_actor_limit,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id = perk_entry_point::apply_sneak_spell,
         .function_type = entry_point_function_type::spell,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_player_magic_slowdown,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_ward_magicka_absorb_percent,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_initial_ingredient_effects_learned,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id = perk_entry_point::purify_alchemy_ingredients,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::filter_activation,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::reference,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::can_dual_cast_spell,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_tempering_health,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::misc_item, // TODO: change this
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_enchantment_power,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Enchantment",
               .type = dovah::form_type::enchantment,
            },
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::misc_item, // TODO: change this
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_soul_percent_captured_to_weapon,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::weapon,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_soul_gem_enchanting,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Enchantment",
               .type = dovah::form_type::enchantment,
            },
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::misc_item, // TODO: change this
            },
         },
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_num_enchantments_allowed,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::set_activate_label,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::reference,
            },
         },
         .function_type = entry_point_function_type::localized_string,
      },
      perk_entry_point_info{
         .id = perk_entry_point::mod_shout_okay,
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_poison_dose_count,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Weapon",
               .type = dovah::form_type::weapon,
            },
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::should_apply_placed_item,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::potion,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_armor_rating,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::armor,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_lockpicking_crime_chance,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Locked Ref",
               .type = dovah::form_type::reference,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_ingredients_harvested,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Item",
               .type = dovah::form_type::ingredient,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_spell_range,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_alchemy_potions_created,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Spell",
               .type = dovah::form_type::spell,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::mod_lockpicking_key_reward_chance,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Locked Ref",
               .type = dovah::form_type::reference,
            },
         },
      },
      perk_entry_point_info{
         .id   = perk_entry_point::allow_mount_actor,
         .args = {
            perk_entry_point_info::perk_owner_condition_subject,
            perk_entry_point_info::condition_subject{
               .name = "Target",
               .type = dovah::form_type::actor,
            },
         },
      },
   };

   static_assert(
      []() -> bool {
         for (size_t i = 0; i < all_perk_entry_points.size(); ++i) {
            auto& info = all_perk_entry_points[i];
            if ((size_t)info.id != i)
               return false;
         }
         return true;
      }(),
      "Perk entry point info must be in the same order as the underlying enum, and must not have gaps."
   );
   static_assert(all_perk_entry_points.size() == 0x5C, "The perk entry point info list shouldn't be missing anything.");
}