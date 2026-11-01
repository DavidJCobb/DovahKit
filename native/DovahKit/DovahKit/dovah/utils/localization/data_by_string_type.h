#pragma once
#include <utility> // std::unreachable
#include "dovah/types/localized_string_type.h"

namespace dovah::utils::localization {
   template<typename T>
   struct data_by_string_type {
      T description;
      T general;
      T info;

      constexpr const T& for_type(localized_string_type v) const noexcept {
         switch (v) {
            case localized_string_type::description: return this->description;
            case localized_string_type::general:     return this->general;
            case localized_string_type::info:        return this->info;
         }
         std::unreachable();
      }
      constexpr T& for_type(localized_string_type v) noexcept {
         return const_cast<T&>(std::as_const(*this).for_type(v));
      }
   };
}