#pragma once
#include <type_traits>

namespace cobb::concepts {
   namespace impl::_all_unique_types {
      template<typename Needle, typename... Haystack>
      constexpr size_t count_in = []() {
         size_t count = 0;
         ((std::is_same_v<Needle, Haystack> ? ++count, 0 : 0), ...);
         return count;
      }();
   }

   template<typename... Types>
   concept all_unique_types = ((impl::_all_unique_types::count_in<Types, Types...> == 1) && ...);
}