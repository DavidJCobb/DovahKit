#include "move_camera.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

#include "../../core.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void move_camera::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      response res;

      auto& vec = (input.button.press_type == worldinput::button_press_type::hold) ? res.held : res.instant;
      vec = o.magnitudes;
      if (o.range.has_value()) {
         if (!input.has_range)
            return;
         o.range.value().scale(vec, input);
      }
      if (o.frame != reference_frame::world) {
         auto& worldedit_core = core::get();

         auto frame_mat = worldedit_core.get_frame_rotation_matrix(o.frame);
         vec = frame_mat * vec.to_struct<glm::vec3>();
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void move_camera::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }
}