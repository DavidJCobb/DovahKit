#pragma once
#include <array>
#include <cstdint>
#include "../core.h"

namespace dovah {
   enum class game_setting_type {
      none = -1,
      boolean,
      float32,
      integer,
      string,
   };

   class game_setting_definition {
      public:
         const char*       name;
         game_setting_type type;
         const char* default_string = nullptr;
         union {
            float   f;
            int32_t i;
         } default_value;
         //
         game_setting_definition(game_setting_type t) : type(t) {}
         game_setting_definition(const char* n, float value);
         game_setting_definition(const char* n, int32_t value);
         game_setting_definition(const char* n, const char* value);
         //
         static const game_setting_definition& lookup(const char* name) noexcept;
         //
         inline bool is_none() const noexcept { return this->type == game_setting_type::none; }
   };

   extern std::array<game_setting_definition, 2957> game_settings;
   }