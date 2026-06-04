#include "./game_localized_strings.h"
#include <array>
#include <string_view>
#include <utility> // std::pair
#include "dovah/localized_strings.h"
#include "dovah/utils/get_user_language_name.h"
#include "./character_encoding.h"
#include "./convert_string.h"

namespace {
   using character_encoding = dovahkit::subsystems::game_localized_strings::character_encoding;

   std::array<std::pair<dovah::localization_language, character_encoding>, 19> _language_to_encoding_map = {{
      { dovah::localization_language::arabic,    character_encoding::windows_1256 },
      { dovah::localization_language::chinese,   character_encoding::utf_8 },
      { dovah::localization_language::czech,     character_encoding::windows_1250 },
      { dovah::localization_language::danish,    character_encoding::windows_1252 },
      { dovah::localization_language::english,   character_encoding::windows_1252 },
      { dovah::localization_language::finnish,   character_encoding::windows_1252 },
      { dovah::localization_language::french,    character_encoding::windows_1252 },
      { dovah::localization_language::german,    character_encoding::windows_1252 },
      { dovah::localization_language::greek,     character_encoding::windows_1253 },
      { dovah::localization_language::hungarian, character_encoding::windows_1250 },
      { dovah::localization_language::italian,   character_encoding::windows_1252 },
      { dovah::localization_language::japanese,  character_encoding::utf_8 },
      { dovah::localization_language::norwegian, character_encoding::windows_1252 },
      { dovah::localization_language::polish,    character_encoding::windows_1250 },
      { dovah::localization_language::portugese, character_encoding::windows_1252 },
      { dovah::localization_language::russian,   character_encoding::windows_1251 },
      { dovah::localization_language::spanish,   character_encoding::windows_1252 },
      { dovah::localization_language::swedish,   character_encoding::windows_1252 },
      { dovah::localization_language::turkish,   character_encoding::windows_1254 },
   }};

   std::array<std::pair<dovah::localization_language, std::string_view>, 19> _language_names = {{
      #define CASE(name) { dovah::localization_language::name, #name },
      CASE(arabic)
      CASE(chinese)
      CASE(czech)
      CASE(danish)
      CASE(english)
      CASE(finnish)
      CASE(french)
      CASE(german)
      CASE(greek)
      CASE(hungarian)
      CASE(italian)
      CASE(japanese)
      CASE(norwegian)
      CASE(polish)
      CASE(portugese)
      CASE(russian)
      CASE(spanish)
      CASE(swedish)
      CASE(turkish)
      #undef CASE
   }};
}

namespace dovahkit::subsystems::game_localized_strings {
   core::core() {
      this->set_encoding();
   }
   core::~core() {
   }

   void core::set_encoding(character_encoding enc) noexcept {
      auto prior = this->encoding;
      if (enc == prior)
         return;
      this->encoding = enc;
      emit encodingChanged(prior, this->encoding);
   }
   void core::set_encoding() {
      auto lang_name = dovah::utils::get_user_language_name();
      for (auto& c : lang_name)
         c = tolower(c);

      std::optional<dovah::localization_language> language;
      for (const auto& entry : _language_names) {
         if (entry.second == lang_name) {
            language = entry.first;
            break;
         }
      }
      if (language.has_value()) {
         for (auto& entry : _language_to_encoding_map) {
            if (language.value() == entry.first) {
               this->set_encoding(entry.second);
               return;
            }
         }
      }
      this->set_encoding(character_encoding::utf_8);
   }

   QString core::convert_localized_string(const dovah::localized_string& s) const noexcept {
      character_encoding src_encoding = character_encoding::utf_8;
      if (s.localized != dovah::localization_language::none) {
         for (auto& pair : _language_to_encoding_map) {
            if (pair.first == s.localized) {
               src_encoding = pair.second;
               break;
            }
         }
      }
      if (src_encoding == character_encoding::utf_8) {
         return QString::fromUtf8(s.c_str());
      }
      return convert_narrow_to_qt(s.c_str(), src_encoding);
   }
   void core::assign_localized_string(dovah::localized_string& s, const QString& value) const noexcept {
      if (this->encoding == character_encoding::utf_8) {
         s.value = value.toUtf8();
      } else {
         s.value = convert_qt_to_narrow(value, this->encoding);
      }
      s.localized = dovah::localization_language::none;
   }
}