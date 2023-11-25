#pragma once
#include "./range_input_scales_1D.h"
#include "helpers/bitstreams/round_trip_test.h"
#include "helpers/vector3.h"

namespace dovahkit::subsystems::worldinput::util {

   constexpr void range_input_scales_1D::scale(float& out, const tool_request_cause& input) const {
      if (input.range.axes == range_input_axes::y) {
         //
         // When a range input is passed as a single axis, it's passed as the X-axis; so we 
         // need to route the "X-axis input" to our Y-axis options in this case.
         //
         out *= input.range.x;
         if (this->y == sign::negative)
            out *= -1;
      } else if (input.range.axes == range_input_axes::x) {
         out *= input.range.x;
         if (this->x == sign::negative)
            out *= -1;
      } else {
         cobb::vector3<float> vec{ input.range.x, input.range.y, 0 };
         out *= vec.length();
         if (vec.dot({ 1, 1, 0 }) < 0) { // handle negative-magnitude X and Y
            out *= -1;
         }

         bool neg_x = (this->x == sign::negative);
         bool neg_y = (this->y == sign::negative);
         if (neg_x ^ neg_y)
            out *= -1;
      }
   }
   
   constexpr void range_input_scales_1D::stream(cobb::bitstreams::reader& s) {
      s.stream(
         x,
         y
      );
   }
   constexpr void range_input_scales_1D::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         x,
         y
      );
   }
   static_assert(cobb::bitstreams::round_trip_test<range_input_scales_1D>, "Assert: round-trip bitstream serialization produces correct results.");
}