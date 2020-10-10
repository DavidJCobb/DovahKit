#pragma once

namespace dovah {
   enum class localized_string_type {
      common,      // STRINGS
      description, // DLSTRINGS (all DESC subrecords except the one in LSCR; QUST/CNAM; BOOK/CNAM)
      info,        // ILSTRINGS (all INFO values except INFO/RNAM)
   };
}