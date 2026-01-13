#pragma once
#include <QObject>
#include "helpers/singleton_ex.h"
namespace dovah {
   struct localized_string;
}

namespace dovahkit::subsystems::game_localized_strings {
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      protected:
         core();
         ~core();

      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

      protected:
         std::string encoding;
         std::string language_name;

      public:
         constexpr const std::string& get_encoding() const noexcept { return this->encoding; }
         void set_encoding(const std::string& name) noexcept; // use the Qt names
         void set_encoding(); // pulls the language name from Skyrim.ini and uses that to decide

         QString convert_localized_string(const dovah::localized_string&) const noexcept;
         void assign_localized_string(dovah::localized_string&, const QString&) const noexcept; // sets the localized_string's contained std::string, i.e. only suitable for when saving something with no STRINGS files

      signals:
         void encodingChanged(const std::string& prior, const std::string& after);
   };
};