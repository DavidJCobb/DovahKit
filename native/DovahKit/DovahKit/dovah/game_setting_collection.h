#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include "lu/passkey.h"
#include "dovah/loaded_game_setting.h"
#include "dovah/types/bare_form_id.h"
#include "dovah/types/game_settings/managed_value_variant.h"
namespace dovah {
   namespace load_order_processes::build_form_stubs_from_file {
      class process;
   }
}

namespace dovah {
   class game_setting_collection {
      protected:
         std::unordered_map<std::string, loaded_game_setting> _settings_by_name; // keys are lowercase
         mutable std::mutex _lock; // for multi-threaded loads

      public:
         template<typename Callable>
            requires std::is_invocable_v<Callable, const loaded_game_setting&>
         void for_each_loaded_game_setting(Callable&& callable) const {
            for (auto& pair : this->_settings_by_name) {
               if constexpr (std::is_invocable_r_v<lu::foreach_lambda_result, Callable, const loaded_game_setting&>) {
                  auto result = callable(pair.second);
                  if (result == lu::foreach_lambda_result::stop)
                     break;
               } else {
                  callable(pair.second);
               }
            }
         }

         const loaded_game_setting* get_loaded_game_setting(std::string_view) const noexcept;

      public: // passkeyed
         // Returns: inserted/updated setting; last used form ID, if setting is already present
         std::pair<loaded_game_setting&, bare_form_id> _store_game_setting(
            lu::passkey<load_order_processes::build_form_stubs_from_file::process>,
            const tes::file&   source_file,
            const std::string& name,
            bare_form_id       form_id,
            types::game_settings::managed_value_variant&&
         );
   };
}