#include "./game_setting_collection.h"
#include <cassert>
#include "dovah/facts/game_settings/all_definitions.h"
#include "dovah/facts/game_setting_types.h"

namespace dovah {
   std::pair<loaded_game_setting&, bare_form_id> game_setting_collection::_store_game_setting(
      lu::passkey<load_order_processes::build_form_stubs_from_file::process>,
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
         bool identifiable = false;
         bool valid        = false;
         if (!name.empty()) {
            facts::game_setting_types<true>::for_each_pair([&name, &value, &identifiable, &valid]<typename K, char V>() {
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

      bare_form_id last_form_id = 0;

      auto  guard       = std::lock_guard(this->_lock);
      auto& dst         = this->_settings_by_name[key];
      bool  is_new_file = false;
      if (dst.data.empty()) {
         dst.name       = name;
         dst.definition = facts::game_settings::lookup_by_name(name);
         is_new_file = true;
      } else {
         auto& last_file_info = dst.data.back();
         last_form_id = last_file_info.last_form_id;
         is_new_file  = last_file_info.source_file == &source_file;
         if (!is_new_file) {
            if (form_id) {
               last_file_info.last_form_id = form_id;
            }
            last_file_info.value = std::move(value);
         }
      }
      if (is_new_file) {
         dst.data.push_back({
            .source_file  = &source_file,
            .last_form_id = form_id,
            .value        = std::move(value)
         });
      }
      return { dst, last_form_id };
   }
}