#include "./game_settings_subsystem.h"
#include "dovah/data/game_settings.h"
#include "editor/core.h"

namespace dovahkit::subsystems::game_settings {
   game_setting_value core::get_setting_value(const char* name) {
      dovah::loaded_game_setting loaded;
      if (DovahKitCore::get().get_loaded_game_setting(name, loaded)) {
         switch (loaded.get_type()) {
            using enum dovah::game_setting_type;
            case boolean:
               return loaded.value.b;
            case float32:
               return loaded.value.f;
            case integer:
               return loaded.value.i;
            case string:
               return loaded.value.s;
         }
      }
      auto length = strlen(name);
      for (auto& dfn : dovah::game_settings) {
         if (_strnicmp(name, dfn.name, length) != 0)
            continue;
         switch (dfn.type()) {
            using enum dovah::game_setting_type;
            case boolean:
               return dfn.default_value.b;
            case float32:
               return dfn.default_value.f;
            case integer:
               return dfn.default_value.i;
            case string:
               return dfn.default_value.s;
         }
      }
      return {};
   }
   void core::set_setting_value(const char* name, const game_setting_value& src) {
      if (!name || !name[0])
         return;

      dovah::game_setting_value to_write;
      //
      // TODO: The backend does no type-checking; it doesn't use std::variant.
      //       During Phase 1 Sustain, we should fix that. For now, just abort.
      //
      switch (name[0]) {
         case 'b':
         case 'B':
            if (!std::holds_alternative<bool>(src))
               return;
            to_write.b = std::get<bool>(src);
            break;
         case 'f':
         case 'F':
            if (!std::holds_alternative<float>(src))
               return;
            to_write.f = std::get<float>(src);
            break;
         case 'i':
         case 'I':
            if (!std::holds_alternative<int32_t>(src))
               return;
            to_write.i = std::get<int32_t>(src);
            break;
         case 's':
         case 'S':
            if (!std::holds_alternative<dovah::localized_string>(src))
               return;
            to_write.s = std::get<dovah::localized_string>(src);
            break;
      }

      DovahKitCore::get().edit_game_setting(name, to_write);
   }
}