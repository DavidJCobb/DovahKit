#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace dovah {
   enum class actor_value_type {
      attribute       = 0, // e.g. Health, HealRate
      skill           = 1, // e.g. OneHanded, VampirePerks, WerewolfPerks
      ai_temperament  = 2, // e.g. Aggression, Energy, Morality
      resistance      = 3,
      limb_condition  = 4,
      status          = 5, // e.g. DetectLifeRange, Invisibility, NightEye, Paralysis
      miscellaneous   = 6, // e.g. ArmorPerks, BowSpeedBonus, Fame, Infamy, IgnoreCrippledLimbs, JumpingBonus, LeftItemCharge, [Skill]Mod, [Skill]SkillAdvance, Telekinesis, Variable01, WardPower
   };

   struct actor_value_info {
      struct flag {
         flag() = delete;
         enum type : uint32_t {
            hostile_effects_scale_with_difficulty = 1 <<  1, // used for combat-related magic skills
            special_stat_clamps_as_nonzero        = 1 <<  2, // only does anything if `clamp_as_special_stat` is set
            clamp_as_special_stat                 = 1 <<  3, // range is [0, 10] or, if `special_stat_clamps_as_nonzero` is set, [1, 10]
            clamp_as_skill                        = 1 <<  4, // range is [0, 100]
            can_have_modifiers                    = 1 <<  5, // the value can have "permanent," "temporary," and "damage" modifiers
            base_value_is_dynamic_plus_current    = 1 <<  6, // only if base value is computed from each individual actor.
            base_value_computed_from_actor        = 1 <<  7, // base value computed from each individual actor.
            derived_value_is_ignored_if_manual    = base_value_computed_from_actor,
            enumeration                           = 1 <<  8, // the values are an enumeration -- predefined names, rather than arbitrary numbers
            inverted                              = 1 <<  9, // damaging the AV increases its value; restoring the AV decreases its value. used for AVs for which higher values = bad
            base_value_computed_from_race         = 1 << 11, // base value computed from each individual actor. AI process cached values are updated on race change.
            cannot_be_altered_by_scripts          = 1 << 14, // Papyrus functions throw an error if asked to modify the value
            base_value_is_always_zero             = 1 << 15,
            base_value_is_always_one              = 1 << 16, // used for some multipliers, but not all of them, because Bethesda made some mistakes
            base_value_is_always_one_hundred      = 1 << 17, // used for limb condition AVs
            ai_process_caches_current_value       = 1 << 18,
            ai_process_caches_max_value           = 1 << 19, // similar to the "cache current value" flag, but omitted for AVs that have no maximum (e.g. AI behavior enum AVs)
            protected_by_god_mode                 = 1 << 20, // values on the player cannot be reduced through any means while ToggleGodMode (TGM) is active
            displayed_effect_magnitude_times_one_hundred = 1 << 21,
         };
      };

      struct {
         size_t value_count = 0;
         std::array<std::string_view, 10> values;
      } enumeration;
      uint32_t         flags  = 0;
      uint32_t         formID = 0;
      uint32_t         index  = 0;
      std::string_view name;
      actor_value_type type = actor_value_type::miscellaneous;
   };

   inline constexpr const auto all_actor_value_info = std::array{
      actor_value_info{ // Aggression
         .enumeration = {
            .value_count = 4,
            .values = {
               "Unaggressive",
               "Aggressive",
               "Very Aggressive",
               "Frenzied",
            },
         },
         .flags  = (
            actor_value_info::flag::enumeration | 
            actor_value_info::flag::cannot_be_altered_by_scripts | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x4B0,
         .index  = 0,
         .name   = "Aggression",
         .type   = actor_value_type::ai_temperament,
      },
      actor_value_info{ // Confidence
         .enumeration = {
            .value_count = 5,
            .values = {
               "Cowardly",
               "Cautious",
               "Average",
               "Brave",
               "Foolhardy",
            },
         },
         .flags  = (
            actor_value_info::flag::enumeration | 
            actor_value_info::flag::cannot_be_altered_by_scripts
         ),
         .formID = 0x4B1,
         .index  = 1,
         .name   = "Confidence",
         .type   = actor_value_type::ai_temperament,
      },
      actor_value_info{ // Energy
         .formID = 0x4B2,
         .index  = 2,
         .name   = "Energy",
         .type   = actor_value_type::ai_temperament,
      },
      actor_value_info{ // Morality
         .enumeration = {
            .value_count = 4,
            .values = {
               "Any Crime",
               "Violence Against Enemies",
               "Property Crime Only",
               "No Crime",
            },
         },
         .flags  = (
            actor_value_info::flag::enumeration | 
            actor_value_info::flag::cannot_be_altered_by_scripts | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x4B3,
         .index  = 3,
         .name   = "Morality",
         .type   = actor_value_type::ai_temperament,
      },
      actor_value_info{ // Mood
         .enumeration = {
            .value_count = 9,
            .values = {
               "Neutral",
               "Angry",
               "Fear",
               "Happy",
               "Sad",
               "Surprised",
               "Puzzled",
               "Disgusted",
               "UNDEFINED",
            },
         },
         .flags  = (
            actor_value_info::flag::enumeration | 
            actor_value_info::flag::cannot_be_altered_by_scripts
         ),
         .formID = 0x4B4,
         .index  = 4,
         .name   = "Mood",
         .type   = actor_value_type::ai_temperament,
      },
      actor_value_info{ // Assistance
         .enumeration = {
            .value_count = 3,
            .values = {
               "Helps Nobody",
               "Helps Allies",
               "Helps Friends and Allies",
            },
         },
         .flags  = (
            actor_value_info::flag::enumeration | 
            actor_value_info::flag::cannot_be_altered_by_scripts | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x4B5,
         .index  = 5,
         .name   = "Assistance",
         .type   = actor_value_type::ai_temperament,
      },
      actor_value_info{ // OneHanded
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x44C,
         .index  = 6,
         .name   = "OneHanded",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // TwoHanded
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x44D,
         .index  = 7,
         .name   = "TwoHanded",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Marksman
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x44E,
         .index  = 8,
         .name   = "Marksman",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Block
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x44F,
         .index  = 9,
         .name   = "Block",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Smithing
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x450,
         .index  = 10,
         .name   = "Smithing",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // HeavyArmor
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x451,
         .index  = 11,
         .name   = "HeavyArmor",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // LightArmor
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x452,
         .index  = 12,
         .name   = "LightArmor",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Pickpocket
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x453,
         .index  = 13,
         .name   = "Pickpocket",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Lockpicking
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x454,
         .index  = 14,
         .name   = "Lockpicking",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Sneak
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x455,
         .index  = 15,
         .name   = "Sneak",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Alchemy
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x456,
         .index  = 16,
         .name   = "Alchemy",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Speechcraft
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x457,
         .index  = 17,
         .name   = "Speechcraft",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Alteration
         .flags  = (
            actor_value_info::flag::hostile_effects_scale_with_difficulty | 
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x458,
         .index  = 18,
         .name   = "Alteration",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Conjuration
         .flags  = (
            actor_value_info::flag::hostile_effects_scale_with_difficulty | 
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x459,
         .index  = 19,
         .name   = "Conjuration",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Destruction
         .flags  = (
            actor_value_info::flag::hostile_effects_scale_with_difficulty | 
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x45A,
         .index  = 20,
         .name   = "Destruction",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Illusion
         .flags  = (
            actor_value_info::flag::hostile_effects_scale_with_difficulty | 
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x45B,
         .index  = 21,
         .name   = "Illusion",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Restoration
         .flags  = (
            actor_value_info::flag::hostile_effects_scale_with_difficulty | 
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x45C,
         .index  = 22,
         .name   = "Restoration",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Enchanting
         .flags  = (
            actor_value_info::flag::clamp_as_skill | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x45D,
         .index  = 23,
         .name   = "Enchanting",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // Health
         .flags  = (
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x3E8,
         .index  = 24,
         .name   = "Health",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // Magicka
         .flags  = (
            actor_value_info::flag::can_have_modifiers | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value | 
            actor_value_info::flag::protected_by_god_mode
         ),
         .formID = 0x3E9,
         .index  = 25,
         .name   = "Magicka",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // Stamina
         .flags  = (
            actor_value_info::flag::can_have_modifiers | 
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x3EA,
         .index  = 26,
         .name   = "Stamina",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // HealRate
         .flags  = (
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x3EB,
         .index  = 27,
         .name   = "HealRate",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // MagickaRate
         .flags  = (
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x3EC,
         .index  = 28,
         .name   = "MagickaRate",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // StaminaRate
         .flags  = (
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x3ED,
         .index  = 29,
         .name   = "StaminaRate",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // SpeedMult
         .flags  = (
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x3EE,
         .index  = 30,
         .name   = "SpeedMult",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // InventoryWeight
         .formID = 0x3EF,
         .index  = 31,
         .name   = "InventoryWeight",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // CarryWeight
         .flags  = (
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x3F0,
         .index  = 32,
         .name   = "CarryWeight",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // CritChance
         .flags  = actor_value_info::flag::base_value_computed_from_race,
         .formID = 0x3F1,
         .index  = 33,
         .name   = "CritChance",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // MeleeDamage
         .flags  = actor_value_info::flag::base_value_computed_from_race,
         .formID = 0x3F2,
         .index  = 34,
         .name   = "MeleeDamage",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // UnarmedDamage
         .flags  = actor_value_info::flag::base_value_computed_from_race,
         .formID = 0x3F3,
         .index  = 35,
         .name   = "UnarmedDamage",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // Mass
         .flags  = actor_value_info::flag::base_value_computed_from_race,
         .formID = 0x3F4,
         .index  = 36,
         .name   = "Mass",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // VoicePoints
         .flags  = (
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x3F5,
         .index  = 37,
         .name   = "VoicePoints",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // VoiceRate
         .flags  = (
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x3F6,
         .index  = 38,
         .name   = "VoiceRate",
         .type   = actor_value_type::attribute,
      },
      actor_value_info{ // DamageResist
         .flags  = actor_value_info::flag::base_value_computed_from_race,
         .formID = 0x5CE,
         .index  = 39,
         .name   = "DamageResist",
         .type   = actor_value_type::resistance,
      },
      actor_value_info{ // PoisonResist
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5CF,
         .index  = 40,
         .name   = "PoisonResist",
         .type   = actor_value_type::resistance,
      },
      actor_value_info{ // FireResist
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5D0,
         .index  = 41,
         .name   = "FireResist",
         .type   = actor_value_type::resistance,
      },
      actor_value_info{ // ElectricResist
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5D1,
         .index  = 42,
         .name   = "ElectricResist",
         .type   = actor_value_type::resistance,
      },
      actor_value_info{ // FrostResist
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5D2,
         .index  = 43,
         .name   = "FrostResist",
         .type   = actor_value_type::resistance,
      },
      actor_value_info{ // MagicResist
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5D3,
         .index  = 44,
         .name   = "MagicResist",
         .type   = actor_value_type::resistance,
      },
      actor_value_info{ // DiseaseResist
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5D4,
         .index  = 45,
         .name   = "DiseaseResist",
         .type   = actor_value_type::resistance,
      },
      actor_value_info{ // PerceptionCondition
         .flags  = (
            actor_value_info::flag::can_have_modifiers | 
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::base_value_is_always_one_hundred
         ),
         .formID = 0x5D5,
         .index  = 46,
         .name   = "PerceptionCondition",
         .type   = actor_value_type::limb_condition,
      },
      actor_value_info{ // EnduranceCondition
         .flags  = (
            actor_value_info::flag::can_have_modifiers | 
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::base_value_is_always_one_hundred
         ),
         .formID = 0x5D6,
         .index  = 47,
         .name   = "EnduranceCondition",
         .type   = actor_value_type::limb_condition,
      },
      actor_value_info{ // LeftAttackCondition
         .flags  = (
            actor_value_info::flag::can_have_modifiers | 
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::base_value_is_always_one_hundred
         ),
         .formID = 0x5D7,
         .index  = 48,
         .name   = "LeftAttackCondition",
         .type   = actor_value_type::limb_condition,
      },
      actor_value_info{ // RightAttackCondition
         .flags  = (
            actor_value_info::flag::can_have_modifiers | 
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::base_value_is_always_one_hundred
         ),
         .formID = 0x5D8,
         .index  = 49,
         .name   = "RightAttackCondition",
         .type   = actor_value_type::limb_condition,
      },
      actor_value_info{ // LeftMobilityCondition
         .flags  = (
            actor_value_info::flag::can_have_modifiers | 
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::base_value_is_always_one_hundred
         ),
         .formID = 0x5D9,
         .index  = 50,
         .name   = "LeftMobilityCondition",
         .type   = actor_value_type::limb_condition,
      },
      actor_value_info{ // RightMobilityCondition
         .flags  = (
            actor_value_info::flag::can_have_modifiers | 
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::base_value_is_always_one_hundred
         ),
         .formID = 0x5DA,
         .index  = 51,
         .name   = "RightMobilityCondition",
         .type   = actor_value_type::limb_condition,
      },
      actor_value_info{ // BrainCondition
         .flags  = (
            actor_value_info::flag::can_have_modifiers | 
            actor_value_info::flag::base_value_computed_from_race | 
            actor_value_info::flag::base_value_is_always_one_hundred
         ),
         .formID = 0x5DB,
         .index  = 52,
         .name   = "BrainCondition",
         .type   = actor_value_type::limb_condition,
      },
      actor_value_info{ // Paralysis
         .flags  = (
            actor_value_info::flag::base_value_is_always_zero | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x5DC,
         .index  = 53,
         .name   = "Paralysis",
         .type   = actor_value_type::status,
      },
      actor_value_info{ // Invisibility
         .flags  = (
            actor_value_info::flag::base_value_is_always_zero | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x5DD,
         .index  = 54,
         .name   = "Invisibility",
         .type   = actor_value_type::status,
      },
      actor_value_info{ // NightEye
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5DE,
         .index  = 55,
         .name   = "NightEye",
         .type   = actor_value_type::status,
      },
      actor_value_info{ // DetectLifeRange
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5DF,
         .index  = 56,
         .name   = "DetectLifeRange",
         .type   = actor_value_type::status,
      },
      actor_value_info{ // WaterBreathing
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5E0,
         .index  = 57,
         .name   = "WaterBreathing",
         .type   = actor_value_type::status,
      },
      actor_value_info{ // WaterWalking
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5E1,
         .index  = 58,
         .name   = "WaterWalking",
         .type   = actor_value_type::status,
      },
      actor_value_info{ // IgnoreCrippledLimbs
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5E2,
         .index  = 59,
         .name   = "IgnoreCrippledLimbs",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Fame
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5E3,
         .index  = 60,
         .name   = "Fame",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Infamy
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5E4,
         .index  = 61,
         .name   = "Infamy",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // JumpingBonus
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5E5,
         .index  = 62,
         .name   = "JumpingBonus",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // WardPower
         .flags  = (
            actor_value_info::flag::base_value_is_always_zero | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x5E6,
         .index  = 63,
         .name   = "WardPower",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // RightItemCharge
         .flags  = (
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::protected_by_god_mode
         ),
         .formID = 0x5E7,
         .index  = 64,
         .name   = "RightItemCharge",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // ArmorPerks
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5E8,
         .index  = 65,
         .name   = "ArmorPerks",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // ShieldPerks
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5E9,
         .index  = 66,
         .name   = "ShieldPerks",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // WardDeflection
         .flags  = (
            actor_value_info::flag::base_value_is_always_zero | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x5EA,
         .index  = 67,
         .name   = "WardDeflection",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable01
         .formID = 0x5EB,
         .index  = 68,
         .name   = "Variable01",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable02
         .formID = 0x5EC,
         .index  = 69,
         .name   = "Variable02",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable03
         .formID = 0x5ED,
         .index  = 70,
         .name   = "Variable03",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable04
         .formID = 0x5EE,
         .index  = 71,
         .name   = "Variable04",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable05
         .formID = 0x5EF,
         .index  = 72,
         .name   = "Variable05",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable06
         .formID = 0x5F0,
         .index  = 73,
         .name   = "Variable06",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable07
         .formID = 0x5F1,
         .index  = 74,
         .name   = "Variable07",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable08
         .formID = 0x5F2,
         .index  = 75,
         .name   = "Variable08",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable09
         .formID = 0x5F3,
         .index  = 76,
         .name   = "Variable09",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Variable10
         .formID = 0x5F4,
         .index  = 77,
         .name   = "Variable10",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // BowSpeedBonus
         .flags  = actor_value_info::flag::base_value_computed_from_race,
         .formID = 0x5F5,
         .index  = 78,
         .name   = "BowSpeedBonus",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // FavorActive
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5F6,
         .index  = 79,
         .name   = "FavorActive",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // FavorsPerDay
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5F7,
         .index  = 80,
         .name   = "FavorsPerDay",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // FavorsPerDayTimer
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5F8,
         .index  = 81,
         .name   = "FavorsPerDayTimer",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LeftItemCharge
         .flags  = (
            actor_value_info::flag::ai_process_caches_current_value | 
            actor_value_info::flag::protected_by_god_mode
         ),
         .formID = 0x5F9,
         .index  = 82,
         .name   = "LeftItemCharge",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // AbsorbChance
         .flags  = (
            actor_value_info::flag::base_value_is_always_zero | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x5FA,
         .index  = 83,
         .name   = "AbsorbChance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Blindness
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5FB,
         .index  = 84,
         .name   = "Blindness",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // WeaponSpeedMult
         .flags  = (
            actor_value_info::flag::base_value_is_always_zero | 
            actor_value_info::flag::ai_process_caches_current_value
         ),
         .formID = 0x5FC,
         .index  = 85,
         .name   = "WeaponSpeedMult",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // ShoutRecoveryMult
         .flags  = (
            actor_value_info::flag::inverted | 
            actor_value_info::flag::base_value_is_always_one
         ),
         .formID = 0x5FD,
         .index  = 86,
         .name   = "ShoutRecoveryMult",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // BowStaggerBonus
         .flags  = (
            actor_value_info::flag::base_value_is_always_zero | 
            actor_value_info::flag::ai_process_caches_max_value
         ),
         .formID = 0x5FE,
         .index  = 87,
         .name   = "BowStaggerBonus",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Telekinesis
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x5FF,
         .index  = 88,
         .name   = "Telekinesis",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // FavorPointsBonus
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x600,
         .index  = 89,
         .name   = "FavorPointsBonus",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LastBribedIntimidated
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x601,
         .index  = 90,
         .name   = "LastBribedIntimidated",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LastFlattered
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x602,
         .index  = 91,
         .name   = "LastFlattered",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // MovementNoiseMult
         .flags  = (
            actor_value_info::flag::inverted | 
            actor_value_info::flag::base_value_is_always_one
         ),
         .formID = 0x603,
         .index  = 92,
         .name   = "MovementNoiseMult",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // BypassVendorStolenCheck
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x604,
         .index  = 93,
         .name   = "BypassVendorStolenCheck",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // BypassVendorKeywordCheck
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x605,
         .index  = 94,
         .name   = "BypassVendorKeywordCheck",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // WaitingForPlayer
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x606,
         .index  = 95,
         .name   = "WaitingForPlayer",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // OneHandedMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x607,
         .index  = 96,
         .name   = "OneHandedMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // TwoHandedMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x608,
         .index  = 97,
         .name   = "TwoHandedMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // MarksmanMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x609,
         .index  = 98,
         .name   = "MarksmanMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // BlockMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x60A,
         .index  = 99,
         .name   = "BlockMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // SmithingMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x60B,
         .index  = 100,
         .name   = "SmithingMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // HeavyArmorMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x60C,
         .index  = 101,
         .name   = "HeavyArmorMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LightArmorMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x60D,
         .index  = 102,
         .name   = "LightArmorMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // PickPocketMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x60E,
         .index  = 103,
         .name   = "PickPocketMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LockpickingMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x60F,
         .index  = 104,
         .name   = "LockpickingMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // SneakMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x610,
         .index  = 105,
         .name   = "SneakMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // AlchemyMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x611,
         .index  = 106,
         .name   = "AlchemyMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // SpeechcraftMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x612,
         .index  = 107,
         .name   = "SpeechcraftMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // AlterationMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x613,
         .index  = 108,
         .name   = "AlterationMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // ConjurationMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x614,
         .index  = 109,
         .name   = "ConjurationMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // DestructionMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x615,
         .index  = 110,
         .name   = "DestructionMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // IllusionMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x616,
         .index  = 111,
         .name   = "IllusionMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // RestorationMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x617,
         .index  = 112,
         .name   = "RestorationMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // EnchantingMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x618,
         .index  = 113,
         .name   = "EnchantingMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // OneHandedSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x619,
         .index  = 114,
         .name   = "OneHandedSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // TwoHandedSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x61A,
         .index  = 115,
         .name   = "TwoHandedSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // MarksmanSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x61B,
         .index  = 116,
         .name   = "MarksmanSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // BlockSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x61C,
         .index  = 117,
         .name   = "BlockSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // SmithingSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x61D,
         .index  = 118,
         .name   = "SmithingSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // HeavyArmorSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x61E,
         .index  = 119,
         .name   = "HeavyArmorSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LightArmorSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x61F,
         .index  = 120,
         .name   = "LightArmorSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // PickPocketSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x620,
         .index  = 121,
         .name   = "PickPocketSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LockpickingSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x621,
         .index  = 122,
         .name   = "LockpickingSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // SneakSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x622,
         .index  = 123,
         .name   = "SneakSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // AlchemySkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x623,
         .index  = 124,
         .name   = "AlchemySkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // SpeechcraftSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x624,
         .index  = 125,
         .name   = "SpeechcraftSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // AlterationSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x625,
         .index  = 126,
         .name   = "AlterationSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // ConjurationSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x626,
         .index  = 127,
         .name   = "ConjurationSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // DestructionSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x627,
         .index  = 128,
         .name   = "DestructionSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // IllusionSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x628,
         .index  = 129,
         .name   = "IllusionSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // RestorationSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x629,
         .index  = 130,
         .name   = "RestorationSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // EnchantingSkillAdvance
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x62A,
         .index  = 131,
         .name   = "EnchantingSkillAdvance",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LeftWeaponSpeedMult
         .flags  = actor_value_info::flag::base_value_is_always_zero | actor_value_info::flag::ai_process_caches_current_value,
         .formID = 0x62B,
         .index  = 132,
         .name   = "LeftWeaponSpeedMult",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // DragonSouls
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x62C,
         .index  = 133,
         .name   = "DragonSouls",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // CombatHealthRegenMult
         .flags  = actor_value_info::flag::base_value_is_always_zero | actor_value_info::flag::ai_process_caches_current_value | actor_value_info::flag::displayed_effect_magnitude_times_one_hundred,
         .formID = 0x62D,
         .index  = 134,
         .name   = "CombatHealthRegenMult",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // OneHandedPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x62E,
         .index  = 135,
         .name   = "OneHandedPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // TwoHandedPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x62F,
         .index  = 136,
         .name   = "TwoHandedPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // MarksmanPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x630,
         .index  = 137,
         .name   = "MarksmanPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // BlockPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x631,
         .index  = 138,
         .name   = "BlockPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // SmithingPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x632,
         .index  = 139,
         .name   = "SmithingPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // HeavyArmorPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x633,
         .index  = 140,
         .name   = "HeavyArmorPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LightArmorPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x634,
         .index  = 141,
         .name   = "LightArmorPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // PickPocketPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x635,
         .index  = 142,
         .name   = "PickPocketPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // LockpickingPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x636,
         .index  = 143,
         .name   = "LockpickingPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // SneakPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x637,
         .index  = 144,
         .name   = "SneakPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // AlchemyPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x638,
         .index  = 145,
         .name   = "AlchemyPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // SpeechcraftPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x639,
         .index  = 146,
         .name   = "SpeechcraftPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // AlterationPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x63A,
         .index  = 147,
         .name   = "AlterationPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // ConjurationPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x63B,
         .index  = 148,
         .name   = "ConjurationPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // DestructionPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x63C,
         .index  = 149,
         .name   = "DestructionPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // IllusionPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x63D,
         .index  = 150,
         .name   = "IllusionPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // RestorationPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x63E,
         .index  = 151,
         .name   = "RestorationPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // EnchantingPowerMod
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x63F,
         .index  = 152,
         .name   = "EnchantingPowerMod",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // DragonRend
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x640,
         .index  = 153,
         .name   = "DragonRend",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // AttackDamageMult
         .flags  = actor_value_info::flag::base_value_is_always_one | actor_value_info::flag::ai_process_caches_current_value,
         .formID = 0x641,
         .index  = 154,
         .name   = "AttackDamageMult",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // HealRateMult
         .flags  = actor_value_info::flag::base_value_is_always_one_hundred | actor_value_info::flag::ai_process_caches_current_value,
         .formID = 0x642,
         .index  = 155,
         .name   = "HealRateMult",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // MagickaRateMult
         .flags  = actor_value_info::flag::base_value_is_always_one_hundred | actor_value_info::flag::ai_process_caches_current_value,
         .formID = 0x643,
         .index  = 156,
         .name   = "MagickaRateMult",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // StaminaRateMult
         .flags  = actor_value_info::flag::base_value_is_always_one_hundred | actor_value_info::flag::ai_process_caches_current_value,
         .formID = 0x644,
         .index  = 157,
         .name   = "StaminaRateMult",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // WerewolfPerks
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x645,
         .index  = 158,
         .name   = "WerewolfPerks",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // VampirePerks
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x646,
         .index  = 159,
         .name   = "VampirePerks",
         .type   = actor_value_type::skill,
      },
      actor_value_info{ // GrabActorOffset
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x647,
         .index  = 160,
         .name   = "GrabActorOffset",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // Grabbed
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x648,
         .index  = 161,
         .name   = "Grabbed",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // DEPRECATED05
         .flags  = (1 << 13),
         .formID = 0x649,
         .index  = 162,
         .name   = "DEPRECATED05",
         .type   = actor_value_type::miscellaneous,
      },
      actor_value_info{ // ReflectDamage
         .flags  = actor_value_info::flag::base_value_is_always_zero,
         .formID = 0x64A,
         .index  = 163,
         .name   = "ReflectDamage",
         .type   = actor_value_type::miscellaneous,
      },
   };
   static_assert([]() -> bool {
      for (size_t i = 0; i < all_actor_value_info.size(); ++i)
         if (all_actor_value_info[i].index != i)
            return false;
      return true;
   }(), "The actor value info list must be contiguous.");
}
