#pragma once
#include <type_traits>

namespace cobb {
   template<typename T> concept bitstreamable_primitive =
      (
            std::is_arithmetic_v<T>
         || std::is_enum_v<T>
         || std::is_floating_point_v<T>
         || std::is_same_v<T, bool> // is_arithmetic_v actually includes bool, which is cursed
      ) && std::is_trivially_copyable_v<T>;
}