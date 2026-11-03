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
         struct source_file_info {
            const tes::file* source_file   = nullptr;
            uint32_t         record_offset = 0;
            bare_form_id     form_id       = 0; // may be zero if the GMST's record ID did not resolve to a valid form ID
         };

      public:
         const facts::game_settings::definition* definition = nullptr;
         struct {
            std::string loaded;    // needed separately from the definition so we can track GMSTs with invalid/no names
            std::string lowercase; // for faster lookups from maps, etc.
         } name;
         std::vector<source_file_info> source_files;
      protected:
         struct {
            value_variant           variant;
            localization::use_node* string_use_info = nullptr;
         } _value;

      protected:
         // If the GMST was already defined in this source file, with a valid form ID, then this returns 
         // that form ID. The GMST will be moved from that form ID, so any corresponding form stub for 
         // that form ID will need to be cleaned up.
         bare_form_id _on_record_loaded(
            lu::passkey<game_setting_collection>,
            const tes::file&   source_file,
            uint32_t           record_offset,
            bare_form_id       form_id,
            const std::string& name,
            const std::string& name_lowercase,
            value_variant&&    value
         );
   };
}
