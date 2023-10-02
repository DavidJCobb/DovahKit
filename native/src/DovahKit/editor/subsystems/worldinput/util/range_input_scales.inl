#pragma once
#include "./range_input_scales.h"
#include "helpers/bitstreams/round_trip_test.h"

namespace dovahkit::subsystems::worldinput::util {
   constexpr void range_input_scales::input_axis::scale(cobb::vector3<float>& out, float input) const {
      if (this->sign == sign::negative)
         input = -input;
      switch (this->axis) {
         case axis3D::x:
            out.x *= input;
            break;
         case axis3D::y:
            out.y *= input;
            break;
         case axis3D::z:
            out.z *= input;
            break;
      }
   }

   constexpr void range_input_scales::scale(cobb::vector3<float>& out, const tool_request_cause& input) const {
      this->x.scale(out, input.range.x);
      this->y.scale(out, input.range.y);
   }
   
   constexpr void range_input_scales::stream(cobb::bitstreams::reader& s) {
      s.stream(
         x.axis,
         x.sign,
         y.axis,
         y.sign
      );
   }
   constexpr void range_input_scales::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         x.axis,
         x.sign,
         y.axis,
         y.sign
      );
   }
   static_assert(cobb::bitstreams::round_trip_test<range_input_scales>, "Assert: round-trip bitstream serialization produces correct results.");
}