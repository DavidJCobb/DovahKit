#include "localized_strings.h"

namespace dovah {
   localized_string& localized_string::operator=(const std::string& other) noexcept {
      //
      // TODO: fail silently if we are in a strings file
      //
      this->value  = other;
      this->exists = true;
      return *this;
   }
}