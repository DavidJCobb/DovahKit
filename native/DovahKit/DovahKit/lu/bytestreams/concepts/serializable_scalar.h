#pragma once
#include <type_traits>
#include "../../utils/integral_wrapper.h"

namespace lu::bytestreams::concepts {
   template<typename T>
   concept serializable_scalar = std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T> || utils::integral_wrapper_type<T>;
}