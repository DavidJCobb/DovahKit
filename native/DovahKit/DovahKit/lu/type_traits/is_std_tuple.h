#pragma once
#include <tuple>
#include <type_traits>

namespace lu {
   template<typename T>
   struct is_std_tuple : public std::false_type {};

   template<typename... Types>
   struct is_std_tuple<std::tuple<Types...>> : public std::true_type {};

   template<typename T>
   static constexpr const bool is_std_tuple_v = is_std_tuple<T>::value;
}