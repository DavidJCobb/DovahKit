#include "common.h"
#include <array>

namespace {
   using namespace dovah;
   //
   struct _language_to_enum {
      const char* language = ""; // must be lowercase
      localization_language encoding;
   };
   std::array<_language_to_enum, 19> _language_to_enum_map = {{
      { "arabic",    localization_language::arabic },
      { "chinese",   localization_language::chinese },
      { "czech",     localization_language::czech },
      { "danish",    localization_language::danish },
      { "english",   localization_language::english },
      { "finnish",   localization_language::finnish },
      { "french",    localization_language::french },
      { "german",    localization_language::german },
      { "greek",     localization_language::greek },
      { "hungarian", localization_language::hungarian },
      { "italian",   localization_language::italian },
      { "japanese",  localization_language::japanese },
      { "norwegian", localization_language::norwegian },
      { "polish",    localization_language::polish },
      { "portugese", localization_language::portugese },
      { "russian",   localization_language::russian },
      { "italian",   localization_language::italian },
      { "swedish",   localization_language::swedish },
      { "turkish",   localization_language::turkish },
   }};
}
namespace dovah {
   localization_language language_name_to_localization_enum(const char* name) {
      for (auto& entry : _language_to_enum_map)
         if (_stricmp(entry.language, name) == 0)
            return entry.encoding;
      return localization_language::unknown;
   }
}