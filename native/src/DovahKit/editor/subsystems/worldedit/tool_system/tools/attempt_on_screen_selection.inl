#pragma once
#include "./attempt_on_screen_selection.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void attempt_on_screen_selection::options::stream(cobb::bitstreams::reader& s) {
      s.stream(operation);
   }
   constexpr void attempt_on_screen_selection::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(operation);
   }
   static_assert(cobb::bitstreams::round_trip_test<attempt_on_screen_selection::options>, "Assert: round-trip bitstream serialization produces correct results.");
}
