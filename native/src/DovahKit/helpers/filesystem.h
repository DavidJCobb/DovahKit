#pragma once
#include <filesystem>

namespace cobb {
   enum class filename_validation_result {
      valid,
      missing,
      is_a_path,
      is_current_or_parent_directory, // "." or ".."
      windows_device_name, // CON, NUL, etc.
      illegal_character,
      ends_in_period,
   };

   extern filename_validation_result validate_filename(const std::filesystem::path& filename, bool require_stem = true);

   extern bool filename_has_extension(const std::filesystem::path& filename, const std::initializer_list<const char*> extensions); // case-insensitive, but extensions must be representable in latin-1
}
