#pragma once
#include "dovah/types/localized_string_type.h"

namespace dovah::facts::localized_strings {
   constexpr bool serialized_strings_are_length_prefixed(localized_string_type type) {
      switch (type) {
         case localized_string_type::description:
         case localized_string_type::info:
            return true;
      }
      return false;
   }
}