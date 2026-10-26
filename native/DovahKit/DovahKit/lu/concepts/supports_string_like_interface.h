#pragma once
#include <concepts>
#include <type_traits>

namespace lu::concepts {
   template<typename T, typename CharT>
   concept supports_string_like_interface = requires(T& s, const T& cs, CharT c) {
      typename T::value_type;
      requires std::is_same_v<CharT, typename T::value_type>;
      { cs.c_str() } -> std::same_as<const CharT*>;
      { cs.data() } -> std::same_as<const CharT*>;
      { s.data() } -> std::same_as<CharT*>;
      { cs.size() } -> std::convertible_to<std::size_t>;
      { s[0] } -> std::same_as<CharT&>;
      { cs[0] } -> std::convertible_to<CharT>;
   };
}