#pragma once
#include <cstdint>

namespace dovah {
   enum class localization_language : uint8_t {
      none,
      unknown,
      arabic,    // Windows-1256
      chinese,   // UTF-8
      czech,     // Windows-1250
      danish,    // Windows-1252
      english,   // Windows-1252
      finnish,   // Windows-1252
      french,    // Windows-1252
      german,    // Windows-1252
      greek,     // Windows-1253
      hungarian, // Windows-1250
      italian,   // Windows-1252
      japanese,  // UTF-8
      norwegian, // Windows-1252
      polish,    // Windows-1250
      portugese, // Windows-1252
      russian,   // Windows-1251
      spanish,   // Windows-1252
      swedish,   // Windows-1252
      turkish,   // Windows-1254
   };
   enum class localized_string_type {
      common,      // STRINGS
      description, // DLSTRINGS (all DESC subrecords except the one in LSCR; QUST/CNAM; BOOK/CNAM)
      info,        // ILSTRINGS (all INFO values except INFO/RNAM)
   };
   //
   extern localization_language language_name_to_localization_enum(const char*);
}