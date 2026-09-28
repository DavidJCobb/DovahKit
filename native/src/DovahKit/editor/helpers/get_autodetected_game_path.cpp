#include "./get_autodetected_game_path.h"
#include "dovah/data/game.h"
#include "helpers/windows_registry.h"

namespace editor_helpers {
   extern std::filesystem::path get_autodetected_game_path(dovah::game game) {
      std::wstring value(512, 0);
      const wchar_t* key;
      switch (game) {
         case dovah::game::skyrim_classic:
            key = L"SOFTWARE\\Bethesda Softworks\\Skyrim\\";
            break;
         case dovah::game::skyrim_special:
            key = L"SOFTWARE\\Bethesda Softworks\\Skyrim Special Edition\\";
            break;
         default:
            return {};
      }
      bool success = cobb::windows_registry::get_string_value(cobb::windows_registry::hkey::local_machine, key, L"installed path", value);
      if (success) {
         return value;
      }
      return {};
   }
}