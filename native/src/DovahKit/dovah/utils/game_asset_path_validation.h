#pragma once
#include <string>
#include <type_traits>

namespace dovah::utils {
   //
   // NOTE: This is untested. It's generally better to perform path validation within your 
   // frontend, particularly if your string type of choice explicitly supports Unicode or 
   // has a well-defined encoding.
   //
   struct game_asset_path_validation {
      struct error_flag {
         enum type : uint16_t {
            period_in_folder_name            = 0x001,
            data_prefix_miscapitalized       = 0x002, // path starts with "data" but is not capitalized as "Data" or "data"
            first_folder_is_data_superstring = 0x004, // path starts with "dataaaaa/", etc.
            double_data_prefix               = 0x008, // path starts with "data/data/"
            armoraddon_suffix_missing        = 0x010, // ArmorAddon model path filename does not end in _0 or _1
            armoraddon_underscore_issue      = 0x020, // ArmorAddon model path filename does not end in _0 or _1, but contains other underscores
            wrong_path_separators            = 0x040,
            extension_too_long               = 0x080, // some game lookup functions cap (what they think is) the file extension to 10 bytes, including the dot
            has_non_ascii_characters         = 0x100,
         };
      };
      using error_flags_t = std::underlying_type_t<error_flag::type>;

      std::string   path;
      error_flags_t errors = 0;
      bool          is_armoraddon_mesh = false;
      bool          normalize_first    = false; // normalize letter case and path separator

      void validate();
   };
}