#pragma once
#include <utility> // std::unreachable
#include "dovah/localization/category.h"

namespace dovah::utils::localization {
   template<typename T>
   struct data_by_string_category {
      T description;
      T general;
      T info;

      constexpr const T& for_category(localization::category v) const noexcept {
         switch (v) {
            case localization::category::description: return this->description;
            case localization::category::general:     return this->general;
            case localization::category::info:        return this->info;
         }
         std::unreachable();
      }
      constexpr T& for_category(localization::category v) noexcept {
         return const_cast<T&>(std::as_const(*this).for_type(v));
      }
   };
}