#pragma once
#include <array>
#include <cstdint>
#include "./entry_flag_underlying_type.h"
namespace dovah {
   namespace use_info::entry_flags {
      enum class base : entry_flag_underlying_type;
      enum class base_extra_data : entry_flag_underlying_type;
   }
   enum class form_type : uint8_t;
}

namespace dovah::use_info {
   template<typename T>
   struct entry_flag_type_form_types;

   template<typename T>
   constexpr bool entry_flag_type_pertains_to_form_type(dovah::form_type ft) {
      if constexpr (std::is_same_v<T, entry_flags::base>) {
         return true;
      } else if constexpr (std::is_same_v<T, entry_flags::base_extra_data>) {
         if (dovah::form_type_is_reference(ft))
            return true;
         return ft == dovah::form_type::cell;
      } else {
         constexpr const auto& form_types = entry_flag_type_form_types<T>::value;
         if constexpr (form_types.size() == 1) {
            if constexpr (form_types[0] == form_type::reference) {
               return dovah::form_type_is_reference(ft);
            } else {
               return form_types[0] == ft;
            }
         } else {
            for (const auto candidate : entry_flag_type_form_types<T>::value)
               if (candidate == ft)
                  return true;
         }
         return false;
      }
   }
}