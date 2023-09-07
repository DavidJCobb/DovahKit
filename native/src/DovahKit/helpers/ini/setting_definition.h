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

      // NTTPs must be structural class types, so they can't have std::string, etc., as members.
      using nttp_value_type = std::conditional_t<std::is_same_v<T, std::string>, const char*, T>;

      const char* const name;
      nttp_value_type   initial_value;
      [[no_unique_address]] value_constraint_info<T> constraints;

      constexpr bool is_valid() const noexcept {
         if (!name)
            return false;
         if (value_types::value_of<value_type> != name[0])
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
