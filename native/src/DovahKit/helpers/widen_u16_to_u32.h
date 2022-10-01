#pragma once
#include <cstdint>

namespace cobb {
   extern void widen_u16_to_u32(size_t count, const uint16_t* src, uint32_t* dst);
}