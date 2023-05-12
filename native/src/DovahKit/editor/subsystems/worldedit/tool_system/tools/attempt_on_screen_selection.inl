#pragma once
#include "./attempt_on_screen_selection.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void attempt_on_screen_selection::options::read(options_serialization_version version, cobb::streams::bitreader& stream) {
      stream.read(this->operation);
   }
   constexpr void attempt_on_screen_selection::options::write(cobb::streams::bitwriter& stream) const {
      stream.write(this->operation);
   }
}
