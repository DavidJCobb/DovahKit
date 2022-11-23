#pragma once

namespace cobb {
   constexpr size_t align_offset(size_t value, size_t align_to) {
      if (size_t misalign = value % align_to)
         value += align_to - misalign;
      return value;
   }

   template<size_t AlignTo>
   constexpr size_t align_offset(size_t value) {
      if (size_t misalign = value % AlignTo)
         value += AlignTo - misalign;
      return value;
   }
}
