#include "./perk_entry_point.h"
#include <QCoreApplication>

namespace editor::localize {
   extern QString perk_entry_point(dovah::perk_entry_point v) {
      switch (v) {
         case dovah::perk_entry_point::calc_weapon_damage:
            return QCoreApplication::translate("dovah::perk_entry_point", "Calc Weapon Damage");
         case dovah::perk_entry_point::calc_my_crit_chance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Calc My Crit Chance");
         case dovah::perk_entry_point::calc_my_crit_damage:
            return QCoreApplication::translate("dovah::perk_entry_point", "Calc My Crit Damage");
         case dovah::perk_entry_point::calc_mine_explode_chance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Calc Mine Explode Chance");
         case dovah::perk_entry_point::adjust_limb_damage:
            return QCoreApplication::translate("dovah::perk_entry_point", "Adjust Limb Damage");
         case dovah::perk_entry_point::adjust_book_skill_points:
            return QCoreApplication::translate("dovah::perk_entry_point", "Adjust Book Skill Points");
         case dovah::perk_entry_point::mod_recovered_health:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Recovered Health");
         case dovah::perk_entry_point::get_should_attack:
            return QCoreApplication::translate("dovah::perk_entry_point", "Get Should Attack");
         case dovah::perk_entry_point::mod_buy_prices:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Buy Prices");
         case dovah::perk_entry_point::add_leveled_item_on_death:
            return QCoreApplication::translate("dovah::perk_entry_point", "Add Leveled Item on Death");
         case dovah::perk_entry_point::get_max_carry_weight:
            return QCoreApplication::translate("dovah::perk_entry_point", "Get Max Carry Weight");
         case dovah::perk_entry_point::mod_addiction_chance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Addiction Chance");
         case dovah::perk_entry_point::mod_addiction_duration:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Addiction Duration");
         case dovah::perk_entry_point::mod_positive_chem_duration:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Positive Chem Duration");
         case dovah::perk_entry_point::activate:
            return QCoreApplication::translate("dovah::perk_entry_point", "Activate");
         case dovah::perk_entry_point::ignore_running_during_detection:
            return QCoreApplication::translate("dovah::perk_entry_point", "Ignore Running During Detection");
         case dovah::perk_entry_point::ignore_broken_lock:
            return QCoreApplication::translate("dovah::perk_entry_point", "Ignore Broken Lock");
         case dovah::perk_entry_point::mod_enemy_crit_chance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Enemy Crit Chance");
         case dovah::perk_entry_point::mod_sneak_attack_mult:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Sneak Attack Mult");
         case dovah::perk_entry_point::mod_max_placeable_mines:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Max Placeable Mines");
         case dovah::perk_entry_point::mod_bow_zoom:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Bow Zoom");
         case dovah::perk_entry_point::mod_recover_arrow_chance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Recover Arrow Chance");
         case dovah::perk_entry_point::mod_skill_use:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Skill Use");
         case dovah::perk_entry_point::mod_telekinesis_distance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Telekinesis Distance");
         case dovah::perk_entry_point::mod_telekinesis_damage_mult:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Telekinesis Damage Mult");
         case dovah::perk_entry_point::mod_telekinesis_damage:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Telekinesis Damage");
         case dovah::perk_entry_point::mod_bash_damage:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Bash Damage");
         case dovah::perk_entry_point::mod_power_attack_cost:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Power Attack Cost");
         case dovah::perk_entry_point::mod_power_attack_damage:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Power Attack Damage");
         case dovah::perk_entry_point::mod_outgoing_spell_magnitude:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Outgoing Spell Magnitude");
         case dovah::perk_entry_point::mod_outgoing_spell_duration:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Outgoing Spell Duration");
         case dovah::perk_entry_point::mod_magic_second_av_weight:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Magic Second AV Weight");
         case dovah::perk_entry_point::mod_armor_weight:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Armor Weight");
         case dovah::perk_entry_point::mod_incoming_stagger:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Incoming Stagger");
         case dovah::perk_entry_point::mod_outgoing_stagger:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Outgoing Stagger");
         case dovah::perk_entry_point::mod_attack_damage:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Attack Damage");
         case dovah::perk_entry_point::mod_incoming_damage:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Incoming Damage");
         case dovah::perk_entry_point::mod_target_damage_resist:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Target Damage Resist");
         case dovah::perk_entry_point::mod_spell_cost:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Spell Cost");
         case dovah::perk_entry_point::mod_percent_blocked:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Percent Blocked");
         case dovah::perk_entry_point::mod_shield_deflect_arrow_chance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Shield Deflect Arrow Chance");
         case dovah::perk_entry_point::mod_incoming_spell_magnitude:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Incoming Spell Magnitude");
         case dovah::perk_entry_point::mod_incoming_spell_duration:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Incoming Spell Duration");
         case dovah::perk_entry_point::mod_player_intimidation:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Player Intimidation");
         case dovah::perk_entry_point::mod_player_reputation:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Player Reputation");
         case dovah::perk_entry_point::mod_favor_points:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Favor Points");
         case dovah::perk_entry_point::mod_bribe_amount:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Bribe Amount");
         case dovah::perk_entry_point::mod_detection_light:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Detection Light");
         case dovah::perk_entry_point::mod_detection_movement:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Detection Movement");
         case dovah::perk_entry_point::mod_soul_gem_recharge:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Soul Gem Recharge");
         case dovah::perk_entry_point::set_sweep_attack:
            return QCoreApplication::translate("dovah::perk_entry_point", "Set Sweep Attack");
         case dovah::perk_entry_point::apply_combat_hit_spell:
            return QCoreApplication::translate("dovah::perk_entry_point", "Apply Combat Hit Spell");
         case dovah::perk_entry_point::apply_bash_spell:
            return QCoreApplication::translate("dovah::perk_entry_point", "Apply Bash Spell");
         case dovah::perk_entry_point::apply_reanimate_spell:
            return QCoreApplication::translate("dovah::perk_entry_point", "Apply Reanimate Spell");
         case dovah::perk_entry_point::set_boolean_graph_variable:
            return QCoreApplication::translate("dovah::perk_entry_point", "Set Boolean Animation Graph Variable");
         case dovah::perk_entry_point::mod_spell_casting_sound_event:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Spell Casting Sound Event");
         case dovah::perk_entry_point::mod_pickpocket_chance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Pickpocket Chance");
         case dovah::perk_entry_point::mod_detection_sneak_skill:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Detection Sneak Skill");
         case dovah::perk_entry_point::mod_fall_damage:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Fall Damage");
         case dovah::perk_entry_point::mod_lockpick_sweet_spot:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Lockpick Sweet Spot");
         case dovah::perk_entry_point::mod_sell_prices:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Sell Prices");
         case dovah::perk_entry_point::can_pickpocket_equipped_item:
            return QCoreApplication::translate("dovah::perk_entry_point", "Can Pickpocket Equipped Items");
         case dovah::perk_entry_point::mod_lockpick_level_allowed:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Lockpick Level Allowed");
         case dovah::perk_entry_point::set_lockpick_starting_arc:
            return QCoreApplication::translate("dovah::perk_entry_point", "Set Lockpick Starting Arc");
         case dovah::perk_entry_point::set_progression_picking:
            return QCoreApplication::translate("dovah::perk_entry_point", "Set Progression Picking");
         case dovah::perk_entry_point::set_lockpicks_unbreakable:
            return QCoreApplication::translate("dovah::perk_entry_point", "Set Lockpicks Unbreakable");
         case dovah::perk_entry_point::mod_alchemy_effectiveness:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Alchemy Effectiveness");
         case dovah::perk_entry_point::apply_weapon_swing_spell:
            return QCoreApplication::translate("dovah::perk_entry_point", "Apply Weapon Swing Spell");
         case dovah::perk_entry_point::mod_commanded_actor_limit:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Commanded Actor Limit");
         case dovah::perk_entry_point::apply_sneak_spell:
            return QCoreApplication::translate("dovah::perk_entry_point", "Apply Sneak Spell");
         case dovah::perk_entry_point::mod_player_magic_slowdown:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Player Magic Slowdown");
         case dovah::perk_entry_point::mod_ward_magicka_absorb_percent:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Ward Magicka Absorb %");
         case dovah::perk_entry_point::mod_initial_ingredient_effects_learned:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Initial Ingredient Effects Learned");
         case dovah::perk_entry_point::purify_alchemy_ingredients:
            return QCoreApplication::translate("dovah::perk_entry_point", "Purify Alchemy Ingredients");
         case dovah::perk_entry_point::filter_activation:
            return QCoreApplication::translate("dovah::perk_entry_point", "Filter Activation");
         case dovah::perk_entry_point::can_dual_cast_spell:
            return QCoreApplication::translate("dovah::perk_entry_point", "Can Dual-Cast Spell");
         case dovah::perk_entry_point::mod_tempering_health:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Tempering Health");
         case dovah::perk_entry_point::mod_enchantment_power:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Enchantment Power");
         case dovah::perk_entry_point::mod_soul_percent_captured_to_weapon:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Soul % Captured to Weapon");
         case dovah::perk_entry_point::mod_soul_gem_enchanting:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Soul Gem Enchanting");
         case dovah::perk_entry_point::mod_num_enchantments_allowed:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Num Enchantments Allowed");
         case dovah::perk_entry_point::set_activate_label:
            return QCoreApplication::translate("dovah::perk_entry_point", "Set Activate Label");
         case dovah::perk_entry_point::mod_shout_okay:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Shout Okay");
         case dovah::perk_entry_point::mod_poison_dose_count:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Poison Dose Count");
         case dovah::perk_entry_point::should_apply_placed_item:
            return QCoreApplication::translate("dovah::perk_entry_point", "Should Apply Placed Item");
         case dovah::perk_entry_point::mod_armor_rating:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Armor Rating");
         case dovah::perk_entry_point::mod_lockpicking_crime_chance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Lockpicking Crime Chance");
         case dovah::perk_entry_point::mod_ingredients_harvested:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Ingredients Harvested");
         case dovah::perk_entry_point::mod_spell_range:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Spell Range");
         case dovah::perk_entry_point::mod_alchemy_potions_created:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Alchemy Potions Created");
         case dovah::perk_entry_point::mod_lockpicking_key_reward_chance:
            return QCoreApplication::translate("dovah::perk_entry_point", "Mod Lockpicking Key Reward Chance");
         case dovah::perk_entry_point::allow_mount_actor:
            return QCoreApplication::translate("dovah::perk_entry_point", "Allow Mount Actor");
      }
      return "";
   }
}