#pragma once
#include <string>
#include <variant>
#include "./facts/game_setting_types.h"
#include "./types/bare_form_id.h"
namespace dovah {
   namespace tes {
      class file;
   }
   class game_setting_definition;
}

namespace dovah {
   class loaded_game_setting {
      protected:
         using value_variant = facts::game_setting_types<true>::unpack_types_into<std::variant>;

      public:
         const game_setting_definition* definition  = nullptr;
         const tes::file*               source_file = nullptr; // needed for dealing with localized strings
         std::string   name; // needed separately from the definition so we can track GMSTs with invalid/no names
         bare_form_id  form_id = 0;
         value_variant value; // TODO: make private behind accessors, so you can't set e.g. "fMyCoolFloatSetting" to a bool
   };
}
