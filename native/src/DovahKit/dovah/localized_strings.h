#pragma once
#include <string>
#include "localization/common.h"

namespace dovah {
   struct localized_string { // a string that may or may not have its value stored in a "STRINGS" string table file
      uint32_t    index = 0;
      std::string value;
      bool        exists    = false; // (false) if the subrecord wasn't present in the containing form
      localization_language localized = localization_language::none; // uses "none" if the content wasn't pulled from a STRINGS file
      localized_string_type type      = localized_string_type::common;

      localized_string() {}
      localized_string(localized_string_type t) : type(t) {}

      inline const char* c_str() const noexcept { return this->value.c_str(); }
      inline size_t size() const noexcept { return this->value.size(); }
      inline bool empty() const noexcept { return this->value.empty(); }

      localized_string& operator=(const std::string&) noexcept;
      localized_string& operator=(const localized_string&) noexcept;
   };
}