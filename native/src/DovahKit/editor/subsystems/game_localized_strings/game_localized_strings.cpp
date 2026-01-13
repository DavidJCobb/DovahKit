#include "./game_localized_strings.h"
#include <array>
#include <QTextCodec>
#include "dovah/localized_strings.h"
#include "dovah/utils/get_user_language_name.h"

namespace {
   struct _language_to_encoding {
      const char* language = ""; // must be lowercase
      const char* encoding = "";
   };
   std::array< _language_to_encoding, 19> _language_to_encoding_map = {{
      { "arabic",    "Windows-1256" },
      { "chinese",   "UTF-8" },
      { "czech",     "Windows-1250" },
      { "danish",    "Windows-1252" },
      { "english",   "Windows-1252" },
      { "finnish",   "Windows-1252" },
      { "french",    "Windows-1252" },
      { "german",    "Windows-1252" },
      { "greek",     "Windows-1253" },
      { "hungarian", "Windows-1250" },
      { "italian",   "Windows-1252" },
      { "japanese",  "UTF-8" },
      { "norwegian", "Windows-1252" },
      { "polish",    "Windows-1250" },
      { "portugese", "Windows-1252" },
      { "russian",   "Windows-1251" },
      { "spanish",   "Windows-1252" },
      { "swedish",   "Windows-1252" },
      { "turkish",   "Windows-1254" },
   }};
   
   const char* _fallback_encoding_name_for_language(dovah::localization_language l) {
      switch (l) {
         case dovah::localization_language::arabic:     return "Windows-1256";
         case dovah::localization_language::chinese:    return "UTF-8";
         case dovah::localization_language::czech:      return "Windows-1250";
         case dovah::localization_language::danish:     return "Windows-1252";
         case dovah::localization_language::english:    return "Windows-1252";
         case dovah::localization_language::finnish:    return "Windows-1252";
         case dovah::localization_language::french:     return "Windows-1252";
         case dovah::localization_language::german:     return "Windows-1252";
         case dovah::localization_language::greek:      return "Windows-1253";
         case dovah::localization_language::hungarian:  return "Windows-1250";
         case dovah::localization_language::italian:    return "Windows-1252";
         case dovah::localization_language::japanese:   return "UTF-8";
         case dovah::localization_language::norwegian:  return "Windows-1252";
         case dovah::localization_language::polish:     return "Windows-1250";
         case dovah::localization_language::portugese:  return "Windows-1252";
         case dovah::localization_language::russian:    return "Windows-1251";
         case dovah::localization_language::spanish:    return "Windows-1252";
         case dovah::localization_language::swedish:    return "Windows-1252";
         case dovah::localization_language::turkish:    return "Windows-1254";
      }
      return "Windows-1252";
   }
}

namespace dovahkit::subsystems::game_localized_strings {
   core::core() {
      this->set_encoding();
   }
   core::~core() {
   }

   void core::set_encoding(const std::string& name) noexcept {
      auto prior = this->encoding;
      this->encoding = name;
      emit encodingChanged(prior, this->encoding);
   }
   void core::set_encoding() {
      auto language = dovah::utils::get_user_language_name();
      for (auto& c : language)
         c = tolower(c);
      for (auto& entry : _language_to_encoding_map) {
         if (language == entry.language) {
            this->set_encoding(entry.encoding);
            return;
         }
      }
   }

   QString core::convert_localized_string(const dovah::localized_string& s) const noexcept {
      if (s.localized != dovah::localization_language::none) {
         QTextCodec::ConverterState state;
         auto* codec = QTextCodec::codecForName("UTF-8");
         QString text = codec->toUnicode(s.c_str());
         if (state.invalidChars > 0) {
            codec = QTextCodec::codecForName(_fallback_encoding_name_for_language(s.localized));
            text = codec->toUnicode(s.c_str());
         }
         return text;
      }
      QTextCodec* codec = nullptr;
      if (!this->encoding.empty())
         codec = QTextCodec::codecForName(this->encoding.c_str());
      if (!codec)
         codec = QTextCodec::codecForName("Windows-1252");
      return codec->toUnicode(s.c_str());
   }
   void core::assign_localized_string(dovah::localized_string& s, const QString& value) const noexcept {
      QTextCodec* codec = nullptr;
      if (!this->encoding.empty())
         codec = QTextCodec::codecForName(this->encoding.c_str());
      if (!codec)
         codec = QTextCodec::codecForName("Windows-1252");
      s.value     = codec->fromUnicode(value).data();
      s.localized = dovah::localization_language::none;
   }
}