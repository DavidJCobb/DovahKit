#pragma once
#include "./modify_camera_speed_flags.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void modify_camera_speed_flags::options::read(options_serialization_version version, cobb::streams::bitreader& stream) {
      stream.read(boost, precision);
   }
   constexpr void modify_camera_speed_flags::options::write(cobb::streams::bitwriter& stream) const {
      stream.write(boost, precision);
   }
}
