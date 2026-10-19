#pragma once
#include <type_traits>

namespace lu {
   template<typename T, bool Const>
   using set_const = std::conditional_t<Const, const T, T>;
}
