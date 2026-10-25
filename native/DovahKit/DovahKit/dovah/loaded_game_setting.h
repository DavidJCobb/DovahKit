#pragma once
#include <string>
#include <variant>
#include "./facts/game_setting_types.h"
#include "./types/bare_form_id.h"
namespace dovah {
   namespace facts::game_settings {
      class definition;
   }
   namespace tes {
      class file;
   }
}

namespace dovah {
   class loaded_game_setting {
      protected:
         using value_variant = facts::game_setting_types<true>::unpack_types_into<std::variant>;

      public:
         const facts::game_settings::definition* definition       = nullptr;
         const tes::file*                        last_source_file = nullptr;
         std::string   name; // needed separately from the definition so we can track GMSTs with invalid/no names
         bare_form_id  form_id = 0;
         value_variant value; // TODO: make private behind accessors, so you can't set e.g. "fMyCoolFloatSetting" to a bool
   };
}
