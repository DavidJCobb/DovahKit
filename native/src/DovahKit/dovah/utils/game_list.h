#pragma once
#include <array>
#include "../../helpers/enum_bitfields.h"
#include "../data/game.h"

namespace dovah {
   //
   // Use to give data structures a compile-time-specified bitmask of games.
   //
   // To define a game_list at compile time:
   // 
   //    auto a = game_list();
   //    auto b = game_list::from<game::skyrim_classic>;
   //    auto c = game_list::from<game::skyrim_classic, game:skyrim_special>
   // 
   // To check, at run-time, if a game list contains a given game:
   // 
   //    bool classic = a.contains(game::skyrim_classic);
   //
   using game_list = cobb::simple_enum_bitfield<game, uint8_t>;
}