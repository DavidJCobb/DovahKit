#include "scale_selection.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

#include "../../core.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void scale_selection::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      response res;
      auto& dst = (input.button.press_type == worldinput::button_press_type::hold) ? res.held : res.instant;
      {
         float mod = o.mod;
         if (o.range.has_value()) {
            if (!input.has_range)
               return;

            const auto& o_range = o.range.value();

            if (input.range.axes == worldinput::range_input_axes::y) {
               //
               // When a range input is passed as a single axis, it's passed as the X-axis; so we 
               // need to route the "X-axis input" to our Y-axis options in this case.
               //
               mod *= input.range.x;
               if (o_range.y == sign::negative)
                  mod *= -1;
            } else if (input.range.axes == worldinput::range_input_axes::x) {
               mod *= input.range.x;
               if (o_range.x == sign::negative)
                  mod *= -1;
            } else {
               cobb::vector3<float> vec{ input.range.x, input.range.y, 0 };
               mod *= vec.length();
               if (vec.dot({ 1, 1, 0 }) < 0) { // handle negative-magnitude X and Y
                  mod *= -1;
               }

               bool neg_x = (o_range.x == sign::negative);
               bool neg_y = (o_range.y == sign::negative);
               if (neg_x ^ neg_y)
                  mod *= -1;
            }
         }
         if (o.scale_all_together) {
            dst.unified = mod;
         } else {
            dst.individual = mod;
         }
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void scale_selection::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }
}

#include "vulkan/data/camera_coordinate_change.h"
#include <QDebug>

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void scale_selection::invoke(const response& params) {
      auto& worldedit_core = core::get();

      float individual = params.held.individual + params.instant.individual;
      float unified    = params.held.unified    + params.instant.unified;

      constexpr const float epsilon = 0.0001F;

      qDebug("scale_selection %f and %f", individual, unified);

      if (std::fabs(individual) > epsilon) {
         if (!worldedit_core.try_scale_selection(individual, false))
            return;
      }
      if (std::fabs(unified) > epsilon) {
         if (!worldedit_core.try_scale_selection(unified, true))
            return;
      }
   }
}