#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "../../forms/MagicEffect.h"

namespace dovah {
   class form_stub;
}

namespace dovah::text_replacers {
   class effect_item_handler {
      public:
         form_stub* magic_effect = nullptr;
         int32_t    area         = 0;
         int32_t    duration     = 0;
         float      magnitude    = 0;
         loaded_form_ptr<loaded_forms::MagicEffect> loaded_magic_effect;

         bool simulate_active_effect_list = false;

      public:
         std::optional<std::string> try_token_substitution(std::string_view tag, std::string_view subtag, std::string_view parameter);
   };
}