#pragma once
#include <cstdint>
#include "lu/enums/enum_flags.h"

namespace dovah::bsa {
   enum class archive_header_flag {
      include_directory_names,
      include_filenames,
      compressed_by_default,
      retain_directory_names,  // guides run-time behavior; no effect on file structure
      retain_filenames,        // guides run-time behavior; no effect on file structure
      retain_filename_offsets, // guides run-time behavior; no effect on file structure
      big_endian,              // for Xbox 360
      retain_strings,          // guides run-time behavior; no effect on file structure
      embed_filenames,
      use_xmem_codec,          // indicates use of the Xbox 360-exclusive XMem compression algorithm
   };

   using archive_header_flags = lu::enum_flags<archive_header_flag, 32>;

   static constexpr const auto all_mandatory_archive_header_flags = archive_header_flags::from<
      archive_header_flag::include_directory_names,
      archive_header_flag::include_filenames
   >();
}