#pragma once
#include <type_traits>

namespace cobb {
   //
   // Variation on std::as_const which, given a `T*`, will produce a `const T*` 
   // (but not a `T* const`).
   //

   template<typename T> requires (!std::is_pointer_v<T>)
   constexpr const T& as_const(T& t) noexcept {
      return t;
   }

   template<typename T> requires std::is_pointer_v<T>
   constexpr std::add_pointer_t<std::add_const_t<std::remove_pointer_t<T>>> as_const(T t) noexcept {
      return t;
   }

   template<typename T> requires (!std::is_pointer_v<T>)
      constexpr void as_const(const T&& t) = delete;
}
