#pragma once
#include <array>
#include <string_view>
#include "dovah/localization/language.h"

namespace dovah::facts::localized_strings {
   constexpr const auto all_language_names = []() {
      std::array<std::string_view, localization::language_count> names = {};

      #define X(name) names[(size_t)localization::language::name] = #name;
      X(arabic)
      X(chinese)
      X(czech)
      X(danish)
      X(english)
      X(finnish)
      X(french)
      X(german)
      X(greek)
      X(hungarian)
      X(italian)
      X(japanese)
      X(norwegian)
      X(polish)
      X(portugese)
      X(russian)
      X(spanish)
      X(swedish)
      X(turkish)

      return names;
   }();
}