#include "./convert_string.h"
#include "./narrow_character_set_definitions.h"

namespace dovahkit::subsystems::game_localized_strings {
   extern QString convert_narrow_to_qt(std::string_view src, character_encoding encoding) {
      const size_t size = src.size();

      const auto* charset_ptr = narrow_character_sets::character_set_for_encoding(encoding);
      if (!charset_ptr) {
         return QString::fromLatin1(src.data(), size);
      }
      const auto& charset = *charset_ptr;

      QString dst;
      dst.reserve(size);
      for (size_t i = 0; i < size; ++i) {
         uint8_t c = src[i];
         dst += QChar(charset[c]);
      }
      return dst;
   }
   extern std::string convert_qt_to_narrow(QString src, character_encoding encoding) {
      const size_t size = src.size();

      const auto* charset_ptr = narrow_character_sets::character_set_for_encoding(encoding);
      if (!charset_ptr) {
         return src.toStdString();
      }
      const auto& charset = *charset_ptr;

      std::string dst;
      dst.reserve(size);
      for (size_t i = 0; i < size; ++i) {
         QChar   c = src[i];
         uint8_t d = ' '; // HACK: non-representable characters become spaces
         for (size_t j = 0; j < charset.size(); ++j) {
            if (charset[j] == c) {
               d = j;
               break;
            }
         }
         dst += d;
      }
      return dst;
   }
}