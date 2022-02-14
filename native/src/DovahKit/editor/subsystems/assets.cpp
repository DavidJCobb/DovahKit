#include "assets.h"
#include "../core.h"

namespace dovahkit::subsystems {
   assets::assets() {
   }
   
   dovah::bsa_archived_file* assets::lookup_loose_game_asset(const std::filesystem::path& path) {
      return DovahKitCore::get().lookup_loose_game_asset(path);
   }
   dovah::bsa_archived_file* assets::lookup_game_asset(const std::filesystem::path& path, bool allow_loose_files) {
      return DovahKitCore::get().lookup_game_asset(path, allow_loose_files);
   }
   dovah::compiled_papyrus_script assets::parse_compiled_script(const std::string& scriptname) {
      return DovahKitCore::get().parse_compiled_script(scriptname);
   }
}