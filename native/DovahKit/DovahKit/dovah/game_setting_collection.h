#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include "lu/passkey.h"
#include "dovah/loaded_game_setting.h"
#include "dovah/types/bare_form_id.h"
#include "dovah/types/game_settings/value_variant.h"
namespace dovah {
   namespace load_order_processes::build_form_stubs_from_file {
      class process;
   }
   namespace tes {
      class file;
   }
   class active_load_order;
}

namespace dovah {
   class game_setting_collection {
      public:
         active_load_order& owner;
      protected:
         std::unordered_map<std::string, loaded_game_setting> _settings_by_name; // keys are lowercase
         mutable std::mutex _lock; // for multi-threaded loads

      protected:
         // Exists so `for_each_active_file_game_setting` can check loaded settings' source files 
         // without a circular include of `active_load_order.h`:
         const tes::file* _get_active_file() const noexcept;

      public:
         constexpr game_setting_collection(active_load_order& alo) : owner(alo) {}

         template<typename Callable>
            requires std::is_invocable_v<Callable, const loaded_game_setting&>
         void for_each_active_file_game_setting(Callable&& callable) const;

         template<typename Callable>
            requires std::is_invocable_v<Callable, loaded_game_setting&>
         void for_each_active_file_game_setting(Callable&& callable);

         template<typename Callable>
            requires std::is_invocable_v<Callable, const loaded_game_setting&>
         void for_each_loaded_game_setting(Callable&& callable) const;

         template<typename Callable>
            requires std::is_invocable_v<Callable, loaded_game_setting&>
         void for_each_loaded_game_setting(Callable&& callable);

         // NOTE: Subject to iterator invalidation if settings are added/removed from the collection.
         const loaded_game_setting* get_loaded_game_setting(std::string_view) const noexcept;

      public: // passkeyed
         // Returns: inserted/updated setting; last used form ID, if setting is already present
         std::pair<loaded_game_setting&, bare_form_id> _store_game_setting(
            lu::passkey<load_order_processes::build_form_stubs_from_file::process>,
            const tes::file&   source_file,
            const std::string& name,
            uint32_t           record_offset,
            bare_form_id       form_id,
            types::game_settings::value_variant&&
         );
   };
}

#include "./game_setting_collection.inl"