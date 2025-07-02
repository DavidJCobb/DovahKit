#pragma once
#include <array>
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
      custom = 18, // "Package"
      custom_template = 19, // "Package Template"
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
      react_to_destroyed_object,
      react_to_grenade_or_mine,
      steal_warning,
      pickpocket_warning,
      movement_blocked,
      vampire_feed  = 37,
      cannibal_feed = 38,
   };
}