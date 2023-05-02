#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>
#include "./all_tools.h"

namespace dovahkit::subsystems::worldedit::tools {
   using tool_id = std::conditional_t<
      (all_tools::count < std::numeric_limits<uint8_t>::max() - 1),
      uint8_t,
      std::conditional_t<
         (all_tools::count < std::numeric_limits<uint16_t>::max() - 1),
         uint16_t,
         uint32_t
      >
   >;

   inline constexpr tool_id id_of_none = std::numeric_limits<tool_id>::max();
}