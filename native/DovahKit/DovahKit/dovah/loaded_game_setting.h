#pragma once
#include <string>
#include <variant>
#include <vector>
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
         struct per_file_data {
            const tes::file* source_file  = nullptr;
            bare_form_id     last_form_id = 0; // may be zero if the GMST's record ID did not resolve to a valid form ID
            value_variant    value;            // TODO: make private behind accessors, so you can't set e.g. "fMyCoolFloatSetting" to a bool
         };

      public:
         const facts::game_settings::definition* definition = nullptr;
         std::string                name; // needed separately from the definition so we can track GMSTs with invalid/no names
         std::vector<per_file_data> data;
   };
}
