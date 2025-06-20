#pragma once
#include <cstdint>

namespace dovah {
   enum class package_type : uint8_t {
      invalid = -1,

      find = 0,
      follow,
      escort,
      eat,
      sleep,
      wander = 5,
      travel,
      accompany,
      use_item_at,
      ambush,
      flee_non_combat = 10,
      use_magic,
      sandbox,
      patrol,
      guard,
      dialogue = 15,
      use_weapon,
      find2,
      package = 18,
      package_template = 19,
      activate = 20,
      alarm,
      flee,
      trespass,
      spectator,
      react_to_dead = 25,
      get_up_from_chair,
      do_nothing,
      in_game_dialogue,
      surface,
      search_for_attacker = 30,
      avoid_player = 31,

      // internal use only
      vampire_feed  = 37,
      cannibal_feed = 38,
   };
}