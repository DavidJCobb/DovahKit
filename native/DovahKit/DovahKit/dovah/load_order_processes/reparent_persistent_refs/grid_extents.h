#pragma once
#include <cstdint>
#include <utility> // std::pair

namespace dovah::load_order_processes::reparent_persistent_refs {
   struct grid_extents {
      struct {
         int32_t min = 0;
         int32_t max = 0;
      } x;
      struct {
         int32_t min = 0;
         int32_t max = 0;
      } y;

      constexpr std::pair<int32_t, int32_t> center() const noexcept {
         int32_t gx = (this->x.max - this->x.min) / 2 + this->x.min;
         int32_t gy = (this->y.max - this->y.min) / 2 + this->y.min;
         return { gx, gy };
      }

      constexpr bool contains(int32_t gx, int32_t gy) const noexcept {
         if (gx < this->x.min)
            return false;
         if (gx > this->x.max)
            return false;
         if (gy < this->y.min)
            return false;
         if (gy > this->y.max)
            return false;
         return true;
      }

      constexpr void widen_to_include(int32_t gx, int32_t gy) {
         if (gx < this->x.min)
            this->x.min = gx;
         if (gx > this->x.max)
            this->x.max = gx;
         if (gy < this->y.min)
            this->y.min = gy;
         if (gy > this->y.max)
            this->y.max = gy;
      }
   };
}