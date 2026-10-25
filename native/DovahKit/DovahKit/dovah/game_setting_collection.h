#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include "lu/passkey.h"
#include "dovah/loaded_game_setting.h"
#include "dovah/types/bare_form_id.h"
#include "dovah/types/game_settings/managed_value_variant.h"
namespace dovah {
   namespace load_order_processes::build_form_stubs_from_file::threads {
      class game_settings;
   }
}

namespace dovah {
   class game_setting_collection {
      protected:
         std::unordered_map<std::string, loaded_game_setting> _settings_by_name; // keys are lowercase
         mutable std::mutex _lock; // for multi-threaded loads

      public: // passkeyed
         void _store_game_setting(
            lu::passkey<load_order_processes::build_form_stubs_from_file::threads::game_settings>,
            const tes::file&   source_file,
            const std::string& name,
            bare_form_id       form_id,
            types::game_settings::managed_value_variant&&
         );
   };
}