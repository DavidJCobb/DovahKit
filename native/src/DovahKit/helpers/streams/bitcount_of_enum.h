#pragma once
#include <bit>
#include <concepts>
#include <type_traits>
#include "../type_traits/strip_enum.h"

namespace cobb::streams {
   template<const auto V>
   constexpr const size_t bitcount_of_enum_member = std::bit_width((std::make_unsigned_t<cobb::strip_enum_t<decltype(V)>>)V);

   template<typename T>
   constexpr const size_t bitcount_of_enum = sizeof(T) * 8;
}