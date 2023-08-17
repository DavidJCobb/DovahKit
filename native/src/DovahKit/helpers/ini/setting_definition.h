#pragma once
#include <type_traits>
#include "./types.h"
#include "./value_constraint_info.h"

namespace cobb::ini {
   class category;
}

namespace cobb::ini {
   template<typename T> requires value_types::has_key<T>
   struct setting_definition {
      using value_type = T;

      const char* const name;
      T initial_value;
      [[no_unique_address]] value_constraint_info<T> constraints;

      constexpr bool is_valid() const noexcept {
         if (!name)
            return false;
         if (value_types::value_of<T> != name[0])
            return false;
         if (!constraints.allows(initial_value))
            return false;
         return true;
      }
   };

   template<typename T>
   concept setting_definition_type = requires {
      typename T::value_type;
      requires std::is_same_v<std::remove_cv_t<T>, setting_definition<typename T::value_type>>;
   };
}
