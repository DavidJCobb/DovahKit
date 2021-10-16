#pragma once
#include <string>
#include <type_traits>

namespace dovah::utils {
   struct game_asset_path_validation {
      struct error_flag {
         enum type : uint8_t {
            period_in_folder_name            = 0x01,
            data_prefix_miscapitalized       = 0x02, // path starts with "data" but is not capitalized as "Data" or "data"
            first_folder_is_data_superstring = 0x04, // path starts with "dataaaaa/", etc.
            double_data_prefix               = 0x08, // path starts with "data/data/"
            armoraddon_suffix_missing        = 0x10, // ArmorAddon model path filename does not end in _0 or _1
            armoraddon_underscore_issue      = 0x20, // ArmorAddon model path filename does not end in _0 or _1, but contains other underscores
            wrong_path_separators            = 0x40,
            extension_too_long               = 0x80, // some game lookup functions cap (what they think is) the file extension to 10 bytes, including the dot
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