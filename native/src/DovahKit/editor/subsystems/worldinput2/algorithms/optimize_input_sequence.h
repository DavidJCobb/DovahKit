#pragma once
#include "../input_sequence_flat.h"

namespace dovahkit::subsystems::worldinput2 {
   class input_sequence;
}

namespace dovahkit::subsystems::worldinput2::algorithms {
   constexpr input_sequence_flat optimize_input_sequence(
      const input_sequence& src
   );
}

#include "./optimize_input_sequence.inl"