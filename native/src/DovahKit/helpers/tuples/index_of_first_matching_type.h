#pragma once
#include <tuple>

namespace cobb::tuples {
   namespace impl {
      template<typename Tuple, auto Predicate>
      struct _index_of_first_matching_type;

      template<auto Predicate, typename... Types> requires requires {
         { (Predicate.template operator()<Types>() || ...) } -> std::same_as<bool>;
      }
      struct _index_of_first_matching_type<std::tuple<Types...>, Predicate> {
         static constexpr auto value = []() consteval {
            std::size_t i = 0;
            ((Predicate.template operator()<Types>() || (++i, false)) || ...);
            return i < sizeof...(Types) ? i : (std::size_t)-1;
         }();
      };

      template<auto Predicate>
      struct _index_of_first_matching_type<std::tuple<>, Predicate> {
         static constexpr std::size_t value = (std::size_t)-1;
      };
   }

   template<typename Tuple, auto Predicate>
   constexpr std::size_t index_of_first_matching_type = impl::_index_of_first_matching_type<Tuple, Predicate>::value;
}