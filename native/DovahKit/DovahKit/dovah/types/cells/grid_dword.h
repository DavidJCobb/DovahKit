#pragma once
#include <cstdint>
#include <optional> // just for std::hash

namespace dovah::types::cells {
   struct grid_dword {
      public:
         int16_t y = 0;
         int16_t x = 0;

      public:
         constexpr bool operator==(const grid_dword&) const noexcept = default;

         constexpr explicit operator uint32_t() const noexcept {
            return (uint32_t)this->y << 16 | (uint32_t)this->x;
         }
   };
}

template<>
struct std::hash<dovah::types::cells::grid_dword> {
   size_t operator()(const dovah::types::cells::grid_dword& id) const noexcept {
      return std::hash<uint32_t>{}((uint32_t)id);
   }
};