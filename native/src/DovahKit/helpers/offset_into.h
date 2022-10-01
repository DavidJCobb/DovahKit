#pragma once
#include <cstdint>

namespace cobb {
   template<typename T> constexpr T* offset_into(T* buffer, size_t offset) {
      return (T*)((std::intptr_t)buffer + offset);
   }
}
