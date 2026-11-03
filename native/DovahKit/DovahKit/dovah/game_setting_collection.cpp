#include "./game_setting_collection.h"
#include <cassert>
#include "dovah/facts/game_settings/all_definitions.h"
#include "dovah/facts/game_setting_types.h"

namespace dovah {
   const loaded_game_setting* game_setting_collection::get_loaded_game_setting(std::string_view name) const noexcept {
      std::string key;
      {
         key = name;
         for (auto& c : key)
            c = std::tolower(c);
      }
      auto it = this->_settings_by_name.find(key);
      if (it == this->_settings_by_name.end())
         return nullptr;
      return &it->second;
   }

   std::pair<loaded_game_setting&, bare_form_id> game_setting_collection::_store_game_setting(
      lu::passkey<load_order_processes::build_form_stubs_from_file::process>,
      const tes::file&   source_file,
      const std::string& name,
      uint32_t           record_offset,
      bare_form_id       form_id,
      types::game_settings::value_variant&& value
   ) {
      #if _DEBUG
      {
         //
         // Verify that the setting name properly identifies the setting type.
         //
         bool identifiable = false;
         bool valid        = false;
         if (!name.empty()) {
            facts::game_setting_types::for_each_pair([&name, &value, &identifiable, &valid]<typename K, char V>() {
               bool is_this_type = name[0] == V || name[0] == (V - 0x20); // case-insensitive char check
               if (is_this_type)
                  identifiable = true;
               if (std::holds_alternative<K>(value)) {
                  valid = is_this_type;
               }
            });
         }
         if (identifiable) {
            assert(!!valid);
         }
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
      dst._owner = this; // in case newly-created

      bare_form_id moved_from_form_id = dst._on_record_loaded({}, source_file, record_offset, form_id, name, key, std::move(value));
      return { dst, moved_from_form_id };
   }
}