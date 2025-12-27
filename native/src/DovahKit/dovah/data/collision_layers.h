#pragma once
#include <array>

namespace dovah {
   enum class collision_layer {
      unidentified           =  0,
      statik                 =  1,
      anim_static            =  2, anim_statik = anim_static,
      transparent            =  3,
      clutter                =  4, // floats on water
      weapon                 =  5,
      projectile             =  6,
      spell                  =  7,
      biped                  =  8, // actor
      trees                  =  9,
      props                  = 10,
      water                  = 11,
      trigger                = 12,
      terrain                = 13,
      trap                   = 14,
      non_collidable         = 15,
      cloud_trap             = 16,
      ground                 = 17, // no sound on collision
      portal                 = 18,
      debris_small           = 19,
      debris_large           = 20,
      acoustic_space         = 21,
      actor_zone             = 22,
      projectile_zone        = 23,
      gas_trap               = 24,
      shell_casing           = 25,
      transparent_small      = 26,
      invisible_wall         = 27,
      transparent_small_anim = 28,
      ward                   = 29, // as in, the shield emitted by a Ward spell
      character_controller   = 30,
      stair_helper           = 31,
      dead_biped             = 32,
      biped_sans_char_controller = 33,
      avoid_box              = 34,
      collision_box          = 35,
      camera_sphere          = 36,
      door_detection         = 37,
      cone_projectile        = 38,
      camera_pick            = 39,
      item_pick              = 40,
      line_of_sight          = 41,
      path_pick              = 42,
      custom_pick_1          = 43,
      custom_pick_2          = 44,
      spell_explosion        = 45,
      dropping_pick          = 46,
      null                   = 47,
      trigger_falling_trap   = 48,
      navcut                 = 49,
      critter                = 50,
      spell_trigger          = 51,
      living_and_dead_actors = 52,
      detection              = 53,
      trap_trigger           = 54,
   };

   static constexpr const auto all_collision_layers = []() {
      std::array<collision_layer, 55> out = {};
      for (size_t i = 0; i < out.size(); ++i)
         out[i] = (collision_layer)i;
      return out;
   }();
}