#pragma once

namespace dovah {
   // These are in the same order as the skills' respective ActorValues. Several 
   // values in the file format serialize the skills' raw ActorValue indices, but 
   // some serialize the skills with zero-based indices -- yet still in AV order.
   enum class skill {
      one_handed,
      two_handed,
      archery,
      block,
      smithing,
      heavy_armor,
      light_armor,
      pickpocket,
      lockpicking,
      sneak,
      alchemy,
      speech,
      alteration,
      conjuration,
      destruction,
      illusion,
      restoration,
      enchanting,
   };
   constexpr const size_t skill_count = (size_t)skill::enchanting + 1;

   // Index of the first skill's ("One Handed") actor value.
   constexpr const size_t first_skill_actor_value_index = 6;
}