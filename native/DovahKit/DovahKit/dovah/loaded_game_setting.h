#pragma once
#include <string>
#include <variant>
#include "./types/bare_form_id.h"
#include "./types/game_settings/managed_value_variant.h"
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
         using value_variant = types::game_settings::managed_value_variant;

      public:
         const facts::game_settings::definition* definition       = nullptr;
         const tes::file*                        last_source_file = nullptr;
         std::string   name; // needed separately from the definition so we can track GMSTs with invalid/no names
         bare_form_id  form_id = 0;
         value_variant value; // TODO: make private behind accessors, so you can't set e.g. "fMyCoolFloatSetting" to a bool
   };
}
