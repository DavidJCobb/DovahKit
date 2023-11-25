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
            o.range.value().scale(mod, input);
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