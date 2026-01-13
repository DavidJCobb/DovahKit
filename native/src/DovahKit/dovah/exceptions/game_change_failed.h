#pragma once
#include <stdexcept>
#include "../game_change_failure_reason.h"
#include "../data/game.h"

namespace dovah::exceptions {
   class game_change_failed : public std::runtime_error {
      public:
         using error_code = game_change_failure_reason;

      public:
         game_change_failed(error_code ec, game f, game t) : std::runtime_error("Failed to change the game."), code(ec), from(f), to(t) {}
         
         game_change_failure_reason code;
         game from;
         game to;
   };
}