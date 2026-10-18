#pragma once
#include <cstdint>

namespace dovah::tes_file_writing {
   struct worldspace_cell_offset_state {
      uint32_t offset_within_record = 0; // offset of the OFST subrecord header within the WRLD record
      struct {
         struct {
            int32_t x = 0;
            int32_t y = 0;
         } min;
         uint32_t width  = 0;
         uint32_t height = 0;
      } grid;
      
      constexpr size_t grid_coord_to_index(int16_t gx, int16_t gy) const noexcept {
         if (!this->grid.width || !this->grid.height)
            return (size_t)-1;
         gx -= this->grid.min.x;
         gy -= this->grid.min.y;
         if (gx >= this->grid.width || gy >= this->grid.height)
            return (size_t)-1;
         return gx + (gy * this->grid.width);
      }
      constexpr size_t subrecord_body_size() const noexcept {
         return sizeof(uint32_t) * (this->grid.width * this->grid.height);
      }
   };
}