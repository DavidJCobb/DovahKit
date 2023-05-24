#pragma once
#include <bit>
#include "./debug_print.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void debug_print::options::stream(cobb::bitstreams::reader& s) {
      s.stream<std::bit_width(max_length)>(this->text);
   }
   constexpr void debug_print::options::stream(cobb::bitstreams::writer& s) const {
      s.stream<std::bit_width(max_length)>(this->text);
   }
   static_assert(cobb::bitstreams::round_trip_test<debug_print::options>, "Assert: round-trip bitstream serialization produces correct results.");
}
