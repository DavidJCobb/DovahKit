#pragma once
#include "./modify_camera_speed_flags.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void modify_camera_speed_flags::options::stream(cobb::bitstreams::reader& s) {
      s.stream(boost, precision);
   }
   constexpr void modify_camera_speed_flags::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(boost, precision);
   }
   static_assert(cobb::bitstreams::round_trip_test<modify_camera_speed_flags::options>, "Assert: round-trip bitstream serialization produces correct results.");
}
