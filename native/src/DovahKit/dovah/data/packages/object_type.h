#pragma once
#include <cstdint>

namespace dovah::packages {
   enum class object_type : uint32_t {
      none,
      activators,
      armor,
      books,
      clothing,
      containers,
      doors,
      ingredients,
      lights,
      misc,
      flora,
      furniture,
      weapons_any,
      ammo,
      actors_characters, // NPCs. Pre-Skyrim, Actor was the base class of Character and Creature; Skyrim merged them.
      actors_creatures,  // Creatures.
      keys,
      alchemy,
      food,
      all_combat_wearable,
      all_wearable,
      weapons_ranged,
      weapons_melee,
      weapons_none,
      actor_effects_any,
      actor_effects_range_target,
      actor_effects_range_touch,
      actor_effects_range_self,
      actors_any,
   };
}