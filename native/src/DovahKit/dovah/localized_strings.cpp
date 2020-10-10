#include "localized_strings.h"

namespace dovah {
   localized_string& localized_string::operator=(const std::string& other) noexcept {
      this->value     = other;
      this->exists    = true;
      this->localized = false;
      return *this;
   }
   localized_string& localized_string::operator=(const localized_string& other) noexcept {
      this->value     = other.value;
      this->index     = other.index;
      this->localized = other.localized;
      this->exists    = other.exists;
      this->type      = other.type;
      return *this;
   }
}