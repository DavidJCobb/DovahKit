#pragma once
#include <array>
#include <cstdint>
#include "../core.h"
#include "../localized_strings.h"

namespace dovah {
   enum class game_setting_type {
      none = -1,
      boolean,
      float32,
      integer,
      string,
   };

   extern game_setting_type get_game_setting_type_from_name(const char* name);

   struct game_setting_value {
      union {
         bool    b;
         float   f;
         int32_t i = 0;
      };
      localized_string s;
   };

   class game_setting_definition {
      public:
         const char*        name = "";
         game_setting_type  type = game_setting_type::none;
         game_setting_value default_value;
         //
         game_setting_definition(game_setting_type t) : type(t) {}
         game_setting_definition(const char* n, bool value);
         game_setting_definition(const char* n, float value);
         game_setting_definition(const char* n, int32_t value);
         game_setting_definition(const char* n, const char* value);
         //
         static const game_setting_definition& lookup(const char* name) noexcept;
         //
         inline bool is_none() const noexcept { return this->type == game_setting_type::none; }
   };

   extern const std::array<game_setting_definition, 3561> game_settings;
}