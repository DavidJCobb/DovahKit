#pragma once
#include "./condition.h"
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

namespace dovahkit::subsystems::worldinput2 {
   constexpr bool control_scheme_condition::operator==(const control_scheme_condition& other) const noexcept {
      if (this->mode != other.mode)
         return false;
      return true;
   }

   constexpr void control_scheme_condition::stream(cobb::bitstreams::reader& s) {
      s.stream_bits(3, mode);
   }
   constexpr void control_scheme_condition::stream(cobb::bitstreams::writer& s) const {
      s.stream_bits(3, mode);
   }
}