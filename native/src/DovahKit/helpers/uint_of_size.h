#pragma once
#include <cstdint>
#include <type_traits>

namespace cobb {
   template<size_t Size> requires (Size <= 8)
   using uint_of_size = std::conditional_t<
      (Size > 4),
      uint64_t,
      std::conditional_t<
         (Size > 2),
         uint32_t,
         std::conditional_t<(Size == 2), uint16_t, uint8_t>
      >
   >;
}
