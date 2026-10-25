#pragma once
#include <string_view>
#include <variant>
#include "lu/enums/enum_flags.h"
#include "dovah/game.h"

namespace dovah::facts::game_settings {
   class definition {
      public:
         using game_list = lu::enum_flags<game, 8>;

         static constexpr const auto all_games = game_list::from<game::skyrim_classic, game::skyrim_special>();
         static constexpr const auto sse_only  = game_list::from<game::skyrim_special>();

         #define CONSTRUCTOR(_typename_, _char_) \
            constexpr definition(std::string_view name, _typename_ value, game_list games = all_games) : name(name), _default_value({ . _char_ = value }), games(games) { \
               if (name.empty() || name[0] != *#_char_) \
                  throw; \
            }
         CONSTRUCTOR(bool, b)
         CONSTRUCTOR(float, f)
         CONSTRUCTOR(int, i)
         CONSTRUCTOR(std::string_view, s)
         #undef CONSTRUCTOR

      public:
         std::string_view name;
      private:
         union {
            bool             b = false;
            float            f;
            int32_t          i;
            std::string_view s;
         } _default_value;
      public:
         game_list games = game_list::from<game::skyrim_classic, game::skyrim_special>(); // i.e. all

         constexpr std::variant<bool, float, int32_t, std::string_view> get_default_value() const noexcept {
            if (this->name.empty())
               return {};
            switch (this->name[0]) {
               case 'b':
                  return this->_default_value.b;
               case 'f':
                  return this->_default_value.f;
               case 'i':
                  return this->_default_value.i;
               case 's':
                  return this->_default_value.s;
            }
            return {};
         }
   };
}
