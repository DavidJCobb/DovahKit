#pragma once
#include <cstdint>

namespace dovah {
   enum class package_location_type : int32_t {
      near_reference = 0,
      in_cell,
      near_package_start_location,
      near_editor_location,
      object_id,   // any TESBoundObject?
      object_type, // paired with `package_location_object_type_filter` (see below)
      near_linked_reference, // paired with a KYWD for the linked ref
      at_package_location,
      reference_alias,
      location_alias,
      // unknown 10
      // unknown 11
      near_self = 12,
   };

   enum class package_location_object_type_filter : uint32_t {
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