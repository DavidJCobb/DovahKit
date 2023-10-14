#include "./range_control_bind_conflict_resolution.h"
#include "../bind_list.h"
#include "../input_sequence.h"

namespace dovahkit::subsystems::worldinput {
   class bind_list_item;
}

namespace dovahkit::subsystems::worldinput::algorithms {
   extern range_control_conflict_result range_control_bind_conflict_resolution(
      const bind_list_item& a,
      const bind_list_item& b
   ) {
      if (!a.input_sequence.has_range_requirement())
         return {};
      if (!b.input_sequence.has_range_requirement())
         return {};

      const auto& a_range = a.input_sequence.range;
      const auto& b_range = b.input_sequence.range;

      if (a_range.control != b_range.control)
         return {};

      {
         bool a_all_axes = a_range.axes == range_input_axes::all;
         bool b_all_axes = b_range.axes == range_input_axes::all;
         if (!a_all_axes && !b_all_axes) {
            //
            // If both binds target individual axes on the range input, and don't target the 
            // same axis, then they're not in conflict.
            //
            if (a_range.axes != b_range.axes)
               return {};
         } else if (a_all_axes != b_all_axes) {
            //
            // All-Axes/One-Axis Conflict: Both binds target the same range control, but one of 
            // them targets multple axes and the other targets only a specific axis. The latter 
            // should be considered the conflict winner.
            //
            range_control_conflict_result result;
            if (a_all_axes) {
               result.a_outcome.set_axes_blocked(b_range.axes, true);
            } else {
               result.b_outcome.set_axes_blocked(a_range.axes, true);
            }
            return result;
         }
      }

      //
      // If both binds target the same axes on the same range control, then the bind with the more 
      // specific button combination wins.
      //
      auto spec_a = a.input_sequence.specificity();
      auto spec_b = b.input_sequence.specificity();
      if (spec_a != spec_b) {
         range_control_conflict_result result;
         if (spec_a < spec_b) {
            result.a_outcome.set_all_axes_blocked();
         } else {
            result.b_outcome.set_all_axes_blocked();
         }
         return result;
      }

      return {};
   }
}