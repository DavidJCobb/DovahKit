#include "./game_setting_collection.h"
#include <cassert>
#include "dovah/facts/game_settings/all_definitions.h"
#include "dovah/facts/game_setting_types.h"

namespace dovah {
   void game_setting_collection::_store_game_setting(
      lu::passkey<load_order_processes::build_form_stubs_from_file::threads::game_settings>,
      const tes::file&   source_file,
      const std::string& name,
      bare_form_id       form_id,
      types::game_settings::managed_value_variant&& value
   ) {
      #if _DEBUG
      {
         //
         // Verify that the setting name properly identifies the setting type.
         //
         bool valid = false;
         if (!name.empty()) {
            facts::game_setting_types<true>::for_each_pair([&name, &value, &valid]<typename K, char V>() {
               if (std::holds_alternative<K>(value)) {
                  valid = name[0] == V || name[0] == (V - 0x20); // case-insensitive char check
               }
            });
         }
         assert(!!valid);
      }
      #endif

      std::string key;
      {
         key = name;
         for (auto& c : key)
            c = std::tolower(c);
      }

      auto  guard = std::lock_guard(this->_lock);
      auto& dst   = this->_settings_by_name[key];
      dst.last_source_file = &source_file;
      dst.name             = name;
      dst.form_id          = form_id;
      dst.value            = std::move(value);
      dst.definition       = facts::game_settings::lookup_by_name(name);
   }
}