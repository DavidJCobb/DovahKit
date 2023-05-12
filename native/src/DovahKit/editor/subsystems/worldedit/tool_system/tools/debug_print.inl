#pragma once
#include "./debug_print.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void debug_print::options::read(options_serialization_version version, cobb::streams::bitreader& stream) {
      stream.read(this->text);
   }
   constexpr void debug_print::options::write(cobb::streams::bitwriter& stream) const {
      stream.write(this->text);
   }
}
