#pragma once
#include <array>
#include <variant>
#include "../form_types.h"
#include "./actor_values.h"

namespace dovah {
   enum class magic_effect_archetype {
      value_modifier,
      script,
      dispel,
      cure_disease,
      absorb,
      dual_value_modifier,
      calm,
      demoralize,
      frenzy,
      disarm,
      command_summoned,
      invisibility,
      light,
      darkness, // confirmed via RE
      night_eye, // confirmed via RE
      lock,
      open,
      bound_weapon,
      summon_creature,
      detect_life,
      telekinesis,
      paralysis,
      reanimate,
      soul_trap,
      turn_undead,
      guide,
      werewolf_feed,
      cure_paralysis,
      cure_addiction,
      cure_poison,
      concussion,
      value_and_parts,
      accumulate_magnitude,
      stagger,
      peak_value_modifier,
      cloak,
      werewolf, // as in, the transformation
      slow_time,
      rally,
      enhance_weapon,
      spawn_hazard,
      etherealize,
      banish,
      spawn_scripted_ref,
      disguise,
      grab_actor,
      vampire_lord, // as in, the transformation
   };

   struct magic_effect_archetype_info {
      public:
         using associated_item_type = std::variant<
            form_type,
            const actor_value_info*
         >;

      public:
         magic_effect_archetype id;
         std::array<associated_item_type, 2> associated_items = { form_type::none, form_type::none };

         struct {
            bool actor_base_must_be_summonable : 1 = false;
            bool hidden_from_ck_ui             : 1 = false;
         } flags;
   };

   inline constexpr const auto all_magic_effect_archetypes = std::array{
      magic_effect_archetype_info{
         .id = magic_effect_archetype::value_modifier,
         .associated_items = { form_type::actor_value_info, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::script,
         .associated_items = { form_type::script, form_type::none }, // confirmed via RE
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::dispel,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::cure_disease,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::absorb,
         .associated_items = { form_type::actor_value_info, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::dual_value_modifier,
         .associated_items = { form_type::actor_value_info, form_type::actor_value_info },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::calm,
         .associated_items = { &actor_value_info_by_name("Aggression") },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::demoralize,
         .associated_items = { &actor_value_info_by_name("Confidence") },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::frenzy,
         .associated_items = { &actor_value_info_by_name("Aggression") },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::disarm,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::command_summoned,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::invisibility,
         .associated_items = { &actor_value_info_by_name("Invisibility") },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::light,
         .associated_items = { form_type::light, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::darkness,
         .flags = {
            .hidden_from_ck_ui = true,
         },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::night_eye,
         .associated_items = { &actor_value_info_by_name("NightEye") },
         .flags = {
            .hidden_from_ck_ui = true,
         },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::lock,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::open,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::bound_weapon,
         .associated_items = { form_type::weapon, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::summon_creature,
         .associated_items = { form_type::actor_base, form_type::none },
         .flags = {
            .actor_base_must_be_summonable = true, // "Summonable" is an ActorBase flag.
         },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::detect_life,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::telekinesis,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::paralysis,
         .associated_items = { &actor_value_info_by_name("Paralysis") },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::reanimate,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::soul_trap,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::turn_undead,
         .associated_items = { &actor_value_info_by_name("RightMobilityCondition") }, // confirmed via RE. no clue why it's like this, tho.
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::guide,
         .associated_items = { form_type::hazard, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::werewolf_feed,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::cure_paralysis,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::cure_addiction,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::cure_poison,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::concussion,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::value_and_parts,
         .associated_items = { form_type::actor_value_info, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::accumulate_magnitude,
         .associated_items = { form_type::actor_value_info, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::stagger,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::peak_value_modifier,
         .associated_items = { form_type::actor_value_info, form_type::keyword },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::cloak,
         .associated_items = { form_type::spell, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::werewolf,
         .associated_items = { form_type::race, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::slow_time,
         .associated_items = { form_type::script, form_type::none }, // confirmed via RE
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::rally,
         .associated_items = { &actor_value_info_by_name("Confidence") },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::enhance_weapon,
         .associated_items = { &actor_value_info_by_name("WeaponSpeedMult"), form_type::enchantment },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::spawn_hazard,
         .associated_items = { form_type::hazard, form_type::none },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::etherealize,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::banish,
         .associated_items = { &actor_value_info_by_name("Confidence") },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::spawn_scripted_ref,
         .associated_items = { form_type::script, form_type::none }, // confirmed via RE
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::disguise,
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::grab_actor,
         .associated_items = { &actor_value_info_by_name("GrabActorOffset") },
      },
      magic_effect_archetype_info{
         .id = magic_effect_archetype::vampire_lord,
         .associated_items = { form_type::race, form_type::none },
      },
   };
   static_assert([]() -> bool {
      for (size_t i = 0; i < all_magic_effect_archetypes.size(); ++i)
         if ((size_t)all_magic_effect_archetypes[i].id != i)
            return false;
      return true;
   }(), "The magic effect archetype list must be contiguous, must include all archetypes, and must be sorted by ID.");
}