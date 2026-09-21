#include "attempt_on_screen_selection.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"
#include "../../core.h"
#include "../../passkeys/attempt_on_screen_selection.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void attempt_on_screen_selection::request(const tool_request_cause& input, const options_union& raw_options, tool_response_tuple& all_responses) {
      const options& o = raw_options.as<options>();
      //
      response res = {
         .operation = o.operation,
      };
      if (input.button.press_type == worldinput::button_press_type::hold) {
         if (input.has_button) {
            if (!input.button.down_state_changed_this_frame) {
               res.sweep = true;
            }
         } else if (input.has_range) {
            res.sweep = true;
         }
      }
      if (input.raycast.has_value()) {
         const auto& raycast = input.raycast.value();
         if (raycast.hit_position.has_value()) {
            res.hit_position = raycast.hit_position.value();
            res.target       = raycast.target_info.form;
         }
      }
      if (res.sweep) {
         //
         // For a "sweep" selection, we want to only modify an entity's selection state 
         // on the first frame that the cursor sweeps over it.
         //
         auto& worldedit_core = core::get();
         if (res.target) {
            auto* prior_target = worldedit_core._get_selection_swept_ref({});
            if (prior_target == res.target) {
               //
               // Cursor is already over this ref. Make this a no-op operation.
               //
               res.operation = selection_operation::no_op;
            }
            worldedit_core._set_selection_swept_ref({}, res.target); // have Worldedit help us remember current frame ref, for the next frame
         } else {
            worldedit_core._set_selection_swept_ref({}, nullptr); // have Worldedit help us remember current frame ref, for the next frame
         }
      }
      all_responses.merge_member(input, res);
   }
   /*static*/ void attempt_on_screen_selection::request_for_hold_release(const options_union&, tool_response_tuple&) {
      auto& worldedit_core = core::get();
      worldedit_core._set_selection_swept_ref({}, nullptr); // have Worldedit help us remember current frame ref, for a future frame
   }
}

#include "../../core.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void attempt_on_screen_selection::invoke(const response& params) {
      auto& worldedit_core = core::get();

      if (auto* stub = params.target; stub && dovah::form_type_is_reference(stub->form_type)) {
         switch (params.operation) {
            case selection_operation::no_op:
               break;
            case selection_operation::toggle:
               worldedit_core.toggleRefSelectionState(*stub);
               break;
            case selection_operation::add:
            case selection_operation::remove:
               worldedit_core.setRefSelectionState(*stub, params.operation == selection_operation::add);
               break;
            case selection_operation::replace:
               worldedit_core.replaceRefSelection(*stub);
               break;
         }
      }
   }
}