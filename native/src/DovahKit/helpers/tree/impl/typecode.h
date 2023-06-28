#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

namespace cobb::impl::_node {
   template<size_t Count>
   using typecode = std::conditional_t<
      (Count <= std::numeric_limits<std::uint8_t>::max()),
      std::uint8_t,
      std::conditional_t<
         (Count <= std::numeric_limits<std::uint16_t>::max()),
         std::uint16_t,
         std::conditional_t<
            (Count <= std::numeric_limits<std::uint32_t>::max()),
            std::uint32_t,
            std::size_t
         >
      >
   >;
}