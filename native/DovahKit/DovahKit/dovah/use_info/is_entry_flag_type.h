#pragma once

namespace dovah::use_info {
   template<typename T>
   struct is_entry_flag_type {
      static constexpr const bool value = false;
   };

   //
   // Returns true if T is an entry-flag enum type, and if that type has 
   // been #included into the current translation unit.
   //
   template<typename T>
   constexpr const bool is_entry_flag_type_v = is_entry_flag_type<T>::value;
}