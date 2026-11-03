#pragma once
#include <string>
#include <variant>
#include <vector>
#include "lu/passkey.h"
#include "./types/bare_form_id.h"
#include "./types/game_settings/managed_value_variant.h"
namespace dovah {
   namespace facts::game_settings {
      class definition;
   }
   namespace localization {
      class use_node;
   }
   namespace tes {
      class file;
   }
   class game_setting_collection;
}

namespace dovah {
   class loaded_game_setting {
      friend game_setting_collection;
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
      protected:
         std::vector<localization::use_node*> _string_use_info;

      protected:
         // If the GMST was already defined in this source file, with a valid form ID, then this returns 
         // that form ID. The GMST will be moved from that form ID, so any corresponding form stub for 
         // that form ID will need to be cleaned up.
         bare_form_id _on_record_loaded(lu::passkey<game_setting_collection>, const tes::file& source_file, bare_form_id form_id, const std::string& name, value_variant&& value);
   };
}
