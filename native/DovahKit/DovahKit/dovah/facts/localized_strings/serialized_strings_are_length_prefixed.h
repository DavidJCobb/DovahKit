#pragma once
#include "dovah/localization/category.h"

namespace dovah::facts::localized_strings {
   constexpr bool serialized_strings_are_length_prefixed(localization::category type) {
      switch (type) {
         case localization::category::description:
         case localization::category::info:
            return true;
      }
      return false;
   }
}