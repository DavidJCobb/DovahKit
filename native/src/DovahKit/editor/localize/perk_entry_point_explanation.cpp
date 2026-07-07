#include "./perk_entry_point_explanation.h"
#include <QCoreApplication>
#include "./perk_entry_point.h"

#define ENUMERATION_TYPE dovah::perk_entry_point

#define STRING(t) QCoreApplication::translate("perk entry point explanation", t)

namespace editor::localize {
   extern QString perk_entry_point_explanation(ENUMERATION_TYPE v) {
      using enum ENUMERATION_TYPE;

      bool    is_actually_completely_unused = false; // if, per my disassembler, no calls to CalculatePerkData pertain to the entry point
      bool    number_result_treated_as_bool = false;
      bool    player_perk_owner_only        = false; // game only checks the entry point on the player
      bool    obviously_player_only         = false; // e.g. a perk that involves UI interactions
      QString result;
      switch (v) {
         case add_leveled_item_on_death:
            player_perk_owner_only = true;
            result = STRING(
               "<p>Adds a Leveled Item to the Target when they're killed by the Perk Owner. This entry point is only "
               "checked for perks owned by the player.</p>"
            );
            break;
         case adjust_limb_damage:
            return STRING(
               "<p>A Fallout 3 leftover. Modifies the damage dealt to the Perk Owner's limbs. Skyrim still checks "
               "this, but only when the Perk Owner has a Body Part (defined in BodyPartData) that associates an "
               "Actor Value with whatever hardcoded limb ID was hit. The Creation Kit doesn't ordinarily allow you "
               "to tie Body Parts to Actor Values, and Skyrim's hardcoded limb IDs mostly aren't useful (i.e. they're "
               "things like \"Saddle\" and \"Fly Grab\").</p>"
            );
         case apply_bash_spell:
            result = STRING(
               "<p>Applies a spell to the Target when the Perk Owner sucessfully bashes them with a shield or weapon.</p>"
            );
            break;
         case apply_combat_hit_spell:
            result = STRING(
               "<p>Applies a spell to the Target when the Perk Owner sucessfully attacks them with a melee weapon, or "
               "at the instant the Perk Owner fires an arrow at them.</p>"
            );
            break;
         case apply_reanimate_spell:
            result = STRING(
               "<p>Applies a spell to the Target when the Perk Owner sucessfully reanimates them from the dead. "
               "You can check conditions against the spell that was used to reanimate the target.</p>"
            );
            break;
         case apply_sneak_spell:
            result = STRING(
               "<p>Applies a spell to the Perk Owner while they are sneaking.</p>"
            );
            break;
         case apply_weapon_swing_spell:
            result = STRING(
               "<p>Applies a spell to the Perk Owner when the Attacker swings a melee weapon at them. Note that for "
               "power attacks, the entry point runs before the attack actually makes contact.</p>"
            );
            break;
         case calc_mine_explode_chance:
            player_perk_owner_only = true;
            result = STRING(
               "<p>If the Perk Owner is the player and they enter a landmine's trigger radius, this modifies the "
               "percentage chance that the landmine will detonate. The default value is 100.</p>"
            );
            break;
         case calc_my_crit_chance:
            return STRING(
               "<p>Modifies the Perk Owner's percentage chance to land a critical hit on the Target when using a "
               "given Weapon.</p>"
            );
         case calc_my_crit_damage:
            return STRING(
               "<p>Modifies the damage inflicted by the Perk Owner's critical hits on the Target, when those hits "
               "are made with a given Weapon. The base crit damage is determined by the Weapon's settings.</p>"
            );
         case calc_weapon_damage:
            //
            // A hidden perk is used to increase silver weapons' base damage by 20 points when used against 
            // werewolves and the undead.
            //
            result = STRING(
               "<p>Modifies the base damage dealt by the Perk Owner's weapon, i.e. the damage output shown "
               "when examining the weapon in the inventory menu. (This differs from %1, which modifies the "
               "total physical damage dealt.)</p>"
            ).arg(perk_entry_point(dovah::perk_entry_point::mod_attack_damage));
            break;
         case can_dual_cast_spell:
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>Controls whether the Perk Owner can dual-cast spells.</p>"
            );
            break;
         case can_pickpocket_equipped_item:
            player_perk_owner_only        = true;
            obviously_player_only         = true;
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>Controls whether the Perk Owner can pickpocket an equipped item from the Target. (By default, "
               "they can't.) Conditions can be run on the item, to selectively enable pickpocketing of specific "
               "kinds of equipped items.</p>"
            );
            break;
         case filter_activation:
            player_perk_owner_only        = true;
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>Controls whether the Perk Owner can activate the Target, and whether they see an activation "
               "prompt for the Target on the HUD.</p>"
            );
            break;
         case get_max_carry_weight:
            // TODO: UNKNOWN
            break;
         case get_should_attack:
            // TODO: UNKNOWN
            // Influences the behavior of the functionality underlying the GetShouldAttack condition, 
            // but I'd have to RE a whole ton of other checks to figure out the surrounding context.
            break;
         case ignore_broken_lock:
            player_perk_owner_only        = true;
            obviously_player_only         = true;
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>Controls whether the player is able to try to pick broken locks. This is a Fallout 3 leftover; "
               "the game still checks it, but because Skyrim doesn't allow the player to break locks in the first "
               "place, this is effectively unused.</p>"
            );
            break;
         case ignore_running_during_detection:
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>Controls whether the Perk Owner is able to run without becoming easier to detect. Note that "
               "\"running\" and \"sprinting\" are not the same thing; \"running\" refers to an actor moving on land "
               "at or near their fastest non-sprinting speeds.</p>"
            );
            break;
         case set_lockpicks_unbreakable:
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>Controls whether the Perk Owner's current lockpick is impossible to break.</p>"
            );
            break;
         case mod_num_enchantments_allowed:
            return STRING(
               "<p>Adjust the number of enchantments that the Perk Owner is able to apply to a single item at "
               "an Enchanting Table. The default is 1.</p>"
            );
         case mod_armor_weight:
            result = STRING(
               "<p>Modifies the weight of each equipped piece of armor worn by the Perk Owner.</p>"
            );
            break;
         case mod_attack_damage:
            //
            // The Overdraw perk uses this to scale damage dealt with bows.
            // 
            result = STRING(
               "<p>Modifies the damage inflicted by the Perk Owner's physical attacks when attacking the Target "
               "with a given Weapon. (This differs from %1, which modifies just the weapon's base damage and not "
               "the total damage dealt.)</p>"
            ).arg(perk_entry_point(dovah::perk_entry_point::calc_weapon_damage));
            break;
         case mod_bash_damage:
            return STRING(
               "<p>Modifies the damage that the Perk Owner deals by bashing the Target.</p>"
            );
            break;
         case mod_bribe_amount:
            // TODO: UNKNOWN
            break;
         case mod_detection_light:
            return STRING(
               "<p>Modifies the influence of lighting conditions on how easily the Perk Owner can be detected.</p>"
               "<p>The input value is computed from the actor's light level, detection lines of sight, and various "
               "Game Settings including <code>fDetection<wbr/>NightEye<wbr/>Bonus</code>, <code>iLightLevel<wbr/>Exterior<wbr/>Mod</code>, "
               "<code>iLightLevel<wbr/>Interior<wbr/>Mod</code>, and <code>iLightLevel<wbr/>Max</code>. The final result value (after "
               "applying all Perks) will be adjusted as per the Perk Owner's Sneak and Invisibility actor values, "
               "and rounded to a signed integer.</p>"
            );
         case mod_detection_movement:
            return STRING(
               "<p>Modifies how the influence of the Perk Owner's movement on how easily they can be detected.</p>"
            );
         case mod_favor_points:
            is_actually_completely_unused = true;
            break;
         case mod_incoming_damage:
            return STRING(
               "<p>Modifies how much physical damage the Perk Owner takes from a given Attacker using a given weapon.</p>"
            );
         case mod_incoming_spell_duration:
            result = STRING(
               "<p>Modifies the duration of Magic Effects applied to the Perk Owner.</p>"
            );
            break;
         case mod_incoming_spell_magnitude:
            result = STRING(
               "<p>Modifies the magnitude of Magic Effects applied to the Perk Owner.</p>"
            );
            break;
         case mod_incoming_stagger:
            return STRING(
               "<p>Modifies the amount of stagger sustained by the Perk Owner when struck by a given Attacker.</p>"
            );
         case mod_percent_blocked:
            return STRING(
               "<p>Modifies the percentage of damage that the actor will avoid taking when they block an attack.</p>"
            );
         case mod_player_intimidation:
            return STRING(
               "<p>Modifies how scary the Perk Owner is, and thus how easily they can intimidate the Target.</p>"
               "<p>Use the \"What's This?\" button in the title bar, and click here, to see the relevant "
               "mathematical formulae.</p>"
            );
         case mod_player_reputation:
            is_actually_completely_unused = true;
            break;
         case mod_poison_dose_count:
            //
            // TODO: What exactly is the "Spell" that gets tested here?
            //
            result = STRING(
               "<p>Modifies the number of times the Perk Owner's poisoned weapon will apply its poison on hit, before "
               "the poison wears off.</p>"
            );
            break;
         case mod_power_attack_damage:
            return STRING(
               "<p>Modifies the total damage dealt by the Perk Owner's power attacks with a given Weapon against a given "
               "Target.</p>"
            );
         case mod_power_attack_cost:
            return STRING(
               "<p>Modifies the total Stamina cost of the Perk Owner's power attacks with a given Weapon.</p>"
            );
         case mod_magic_second_av_weight:
            result = STRING(
               "<p>Modifies the effect of \"Dual Value Modifier\"-archetype Magic Effects upon their second Actor Value, "
               "when those Magic Effects (belonging to a given Spell) are applied by the Perk Owner to the Target.</p>"
            );
            break;
         case mod_shield_deflect_arrow_chance:
            return STRING(
               "<p>Modifies the chance that the Perk Owner's shield will deflect an incoming arrow, even when the Perk "
               "Owner isn't actively blocking. The final result will be clamped to the range [0, 100].</p>"
            );
         case mod_shout_okay:
            player_perk_owner_only        = true;
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>Controls whether the Perk Owner can Shout. The default is 1.</p>"
            );
            break;
         case mod_soul_gem_enchanting:
            player_perk_owner_only = true;
            obviously_player_only  = true;
            result = STRING(
               "<p>Modifies the amount of energy Soul Gems provide when the Perk Owner is applying a given Enchantment to "
               "a given weapon at an Enchanting Table.</p>"
            );
            break;
         case mod_soul_gem_recharge:
            player_perk_owner_only = true;
            obviously_player_only  = true;
            return STRING(
               "<p>Modifies the amount of energy Soul Gems provide when the Perk Owner uses them to recharge a weapon's "
               "enchantment.</p>"
            );
         case mod_spell_casting_sound_event:
            //
            // TODO: RE suggests that this is a boolean?
            // 
            // TODO: RE suggests that this is backwards, i.e. 0 creates a detection event and 1 skips?? But it 
            //       still defaults to 1??? Did Bethesda make mistakes when implementing it? Are my tools not 
            //       interpreting the floating-point comparison correctly?
            //
            result = STRING(
               "<p>Scales the noise made when the Perk Owner casts the given Spell. The default is 1.0; change to 0.0 to "
               "prevent NPCs from hearing the spell.</p>"
            );
            break;
         case mod_spell_cost:
            result = STRING(
               "<p>Modifies the amount of Magicka the Perk Owner must spend to cast the given Spell.</p>"
            );
            break;
         case mod_outgoing_spell_duration:
            result = STRING(
               "<p>Modifies the duration of a Magic Effect applied to a given Target when the Perk Owner casts a given Spell.</p>"
            );
            break;
         case mod_outgoing_spell_magnitude:
            result = STRING(
               "<p>Modifies the magnitude of a Magic Effect applied to a given Target when the Perk Owner casts a given Spell.</p>"
            );
            break;
         case mod_target_damage_resist:
            //
            // The Skullcrusher perk sets this to 0.25 in order to make warhammers ignore 75% of armor.
            //
            return STRING(
               "<p>Modifies the Armor Rating of a Target attacked by the Perk Owner.</p>"
            );
         case mod_addiction_chance:
            return STRING(
               "<p>In Fallout 3, this modified the chance of an actor developing an addiction to a chem. Skyrim still "
               "checks this entry point, but it's not yet known whether the relevant codepath ever ends up running "
               "during normal play.</p>"
            );
            break;
         case mod_alchemy_effectiveness:
            return STRING(
               "<p>Modifies the Magic Effect magnitudes of new potions and poisons crafted by the Perk Owner.</p>"
            );
         case mod_armor_rating:
            return STRING(
               "<p>Modifies the Armor Rating of each equipped piece of armor worn by the Perk Owner.</p>"
            );
         case mod_bow_zoom:
            return STRING(
               "<p>Modifies how far the Perk Owner can zoom/aim down sights. The default is 0.0 (no zoom); a value of 1.0 allows "
               "full zoom.</p>"
            );
         case mod_buy_prices:
            return STRING(
               "<p>Modifies how much it costs the Perk Owner to buy items from the Target.</p>"
            );
         case mod_commanded_actor_limit:
            result = STRING(
               "<p>Modifies the number of commanded (e.g. summoned, reanimated) actors that the Perk Owner can control at "
               "once. You can run conditions on the specific Spell being used to command these actors, e.g. to exempt a "
               "specific spell from the usual limits.</p>"
            );
            break;
         case mod_detection_sneak_skill:
            return STRING(
               "<p>Modifies the Perk Owner's Sneak skill when it's used in detection-related calculations.</p>"
            );
         case mod_enchantment_power:
            return STRING(
               "<p>Modifies the magnitude of enchantments that the Perk Owner applies to an item at an Enchanting Table. Not "
               "retroactive.</p>"
            );
         case mod_enemy_crit_chance:
            result = STRING(
               "<p>Modifies the Attacker's chance to land a critical hit on the Perk Owner. If the Attacker has any \"%1\" "
               "Perks, that entry point will run before this one.</p>"
            ).arg(perk_entry_point(dovah::perk_entry_point::calc_my_crit_chance));
            break;
         case mod_fall_damage:
            return STRING(
               "<p>Modifies the damage the Perk Owner takes from long falls.</p>"
            );
         case mod_ingredients_harvested:
            return STRING(
               "<p>Modifies the number of ingredients the Perk Owner gains from harvesting a single Flora or Tree. Conditions "
               "can be run against the ingredient base form.</p>"
            );
         case mod_initial_ingredient_effects_learned:
            return STRING(
               "<p>Modifies how many of an ingredient's Magic Effects the Perk Owner can learn by eating the ingredient.</p>"
            );
         case mod_lockpick_level_allowed:
            is_actually_completely_unused = true;
            break;
         case mod_lockpick_sweet_spot:
            return STRING(
               "<p>Modifies the size of the area that the lockpick must be within in order to move the lock mechanism.</p>"
            );
         case mod_lockpicking_crime_chance:
            return STRING(
               "<p>Modifies the chance that the player will be caught committing a crime when picking locks while detected.</p>"
            );
         case mod_lockpicking_key_reward_chance:
            return STRING(
               "<p>Modifies the percentage chance that the player will be awarded with the key (if there is one) "
               "to a lock if they successfully pick that lock.</p>"
            );
         case mod_max_placeable_mines:
            player_perk_owner_only = true;
            result = STRING(
               "<p>Modifies the number of landmines (e.g. Rune spells) that the Perk Owner can place at once. The default "
               "value is the value of the <code>iMaxPlayerRunes</code> Game Setting.</p>"
            );
            break;
         case mod_pickpocket_chance: // "Modify Max Pickpocket Chance" on CK wiki
            return STRING(
               "<p>Modifies the percentage chance that the Perk Owner will successfully pickpocket the Item from "
               "the Target.</p>"
            );
            break;
         case mod_player_magic_slowdown:
            // TODO: UNKNOWN
            break;
         case mod_positive_chem_duration:
            is_actually_completely_unused = true;
            break;
         case mod_alchemy_potions_created:
            return STRING(
               "<p>Modifies the number of potions (or poisons) created by the Perk Owner when combining a single set "
               "of ingredients at an Alchemy workbench.</p>"
            );
         case mod_recover_arrow_chance:
            return STRING(
               "<p>Modifies the chance that an arrow fired by the Perk Owner into a target will be recoverable from "
               "the target's corpse (via their inventory). The final result value will be compared to a random "
               "integer between 0 and 100.</p>"
            );
         case mod_recovered_health:
            is_actually_completely_unused = true;
            break;
         case mod_sell_prices:
            return STRING(
               "<p>Modifies how much money the Perk Owner makes by selling items to the Target.</p>"
            );
         case mod_skill_use:
            return STRING(
               "<p>Modifies the amount of per-skill XP the Perk Owner gains.</p>"
            );
         case mod_sneak_attack_mult:
            return STRING(
               "<p>Modifies the damage multiplier applied to a successful sneak attack.</p>"
            );
         case mod_soul_percent_captured_to_weapon:
            return STRING(
               "<p>A scaling factor that defaults to 0. When the Perk Owner uses an enchanted weapon to kill a "
               "Target, this scaling factor is multiplied by the target's soul size (i.e. the size of Soul Gem it "
               "would fill if it were Soul Trapped). The resulting amount of charge is used to recharge the Perk "
               "Owner's weapon. As an example, a final result value of 0.05 means that 5% of the victim's soul will "
               "be used to recharge the Perk Owner's weapon.</p>"
            );
         case mod_spell_range:
            result = STRING(
               "<p>Modifies the maximum range of \"Target Location\" spells cast by the Perk Owner.</p>"
            );
            break;
         case mod_telekinesis_damage:
            return STRING(
               "<p>Modifies the base damage dealt by the Perk Owner when they fling an object at a target using a "
               "Telekinesis-archetype spell. The default is the value of the <code>fMagic<wbr/>Telekinesis<wbr/>DamageBase</code> "
               "Game Setting.</p>"
            );
         case mod_telekinesis_damage_mult:
            return STRING(
               "<p>Modifies a multiplier applied to the base damage dealt by the Perk Owner when they fling an object "
               "at a target using a Telekinesis-archetype spell. The default is the value of the <code>fMagic<wbr/>Telekinesis<wbr/>DamageMult</code> "
               "Game Setting.</p>"
            );
         case mod_telekinesis_distance:
            player_perk_owner_only = true;
            return STRING(
               "<p>Modifies the max distance from which the Perk Owner can grab an object using a Telekinesis-archetype "
               "spell. The default value is the value of the <code>fMagic<wbr/>Telekinesis<wbr/>BaseDistance</code> Game "
               "Setting.</p>"
            );
            break;
         case mod_tempering_health:
            return STRING(
               "<p>Modifies how effectively the Perk Owner can temper weapons and armor.</p>"
            );
         case mod_ward_magicka_absorb_percent:
            result = STRING(
               "<p>Modifies how much Magicka the Perk Owner's Wards will restore, when hit by spells. The value is a "
               "multiplier that will be applied to the incoming spell's Magicka cost.</p>"
            );
            break;
         case purify_alchemy_ingredients:
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>Controls whether new potions created by the Perk Owner have negative effects removed, and whether "
               "new poisons have positive effects removed. (Whether a combination of ingredients results in a potion "
               "or a poison depends on whether the costliest Magic Effect among them is Detrimental or not.)</p>"
            );
            break;
         case set_activate_label:
            player_perk_owner_only = true;
            obviously_player_only  = true;
            result = STRING(
               "<p>Changes the text of the activation prompt shown when the Perk Owner aims at the Target.</p>"
            );
            break;
         case set_boolean_graph_variable:
            return STRING(
               "<p>Sets a boolean-type variable in the Perk Owner's animation graph to <code>true</code>.</p>"
            );
         case set_lockpick_starting_arc: // unknown, except that the value appears to be measured in degrees?
            // TODO: UNKNOWN
            break;
         case set_sweep_attack:
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>Controls whether the Perk Owner's attacks with a given melee weapon deal damage to all actors and "
               "objects along the arc of its swing. (The default is for melee attacks to act more like short-range, "
               "hitscan, invisible bullets focused in a single direction.)</p>"
            );
            break;
         case should_apply_placed_item:
            number_result_treated_as_bool = true;
            result = STRING(
               "<p>When the Perk Owner reverse-pickpockets a potion or poison into the Target's inventory, this entry "
               "point controls whether the potion or poison is consumed, applying its effects to the Target, instead "
               "of simply sitting idly in their inventory.</p>"
            );
            break;
      }

      if (player_perk_owner_only && !obviously_player_only) {
         result += QCoreApplication::translate("perk entry point explanation - boilerplate",
            "\n"
            "<p>This entry point is only used when the Perk Owner is the player.</p>"
         );
      }

      if (is_actually_completely_unused) {
         result += QCoreApplication::translate("perk entry point explanation - boilerplate",
            "<p>This appears to be completely unused. No uses of it have been discovered through reverse-engineering.</p>"
         );
      }

      //
      // Explain the nuances of return values:
      //
      if (number_result_treated_as_bool) {
         result += QCoreApplication::translate("perk entry point explanation - boilerplate",
            "\n"
            "<p>The result is a boolean: the described functionality is enabled if the result is a non-zero "
            "number, or disabled otherwise.</p>"
         );
      } else {
         if ((size_t)v < dovah::all_perk_entry_points.size()) {
            const auto& ep_info = dovah::all_perk_entry_points[(size_t)v];
            switch (ep_info.value_type) {
               case dovah::entry_point_value_type::leveled_item:
                  result += QCoreApplication::translate("perk entry point explanation - boilerplate",
                     "\n"
                     "<p>Note that due to how the perk entry point system is designed, only one leveled item can be applied, "
                     "based on the priority of all perks using this entry point.</p>"
                  );
                  break;
               case dovah::entry_point_value_type::spell:
                  result += QCoreApplication::translate("perk entry point explanation - boilerplate",
                     "\n"
                     "<p>The spell must be Fire and Forget/Contact, Constant Effect/Contact, or Constant Effect/Self. Note that "
                     "due to how the perk entry point system is designed, only one spell can be applied, based on the priority "
                     "of all perks using this entry point.</p>"
                  );
                  break;
            }
         }
      }

      return result;
   }

   namespace elaborated {
      extern QString perk_entry_point_explanation(dovah::perk_entry_point v) {
         switch (v) {
            using enum dovah::perk_entry_point;
            case mod_player_intimidation:
               return STRING(
                  (const char*)
                  u8"<p>The player's scariness is computed using this formula, and is the value adjusted by this entry point:</p>"
                  u8"<p>player level \u00D7 (1 + max(-1, (player speech - target speech) \u00F7 100))<sup><code>fIntimidateSpeechcraftCurve</code></sup></p>"
                  u8"<p>The target's bravery is computed by multiplying their level by their confidence multiplier (one "
                  u8"of the game settings whose names start with <code>fIntimidateConfidenceMult</code>).</p>"
                  u8"<p>If (after applying Perks) the player is more scary than the target is brave, then "
                  u8"the player will be able to intimidate the target.</p>"
               );
         }
         return "";
      }
   }
}