#include "attempt_on_screen_selection.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void attempt_on_screen_selection::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_responses) {
      const options& o = raw_options.as<options>();
      //
      response res = {
         .operation = o.operation,
      };
      if (input.has_button) {
         if (input.button.press_type == worldinput::button_press_type::hold && !input.button.down_state_changed_this_frame) {
            res.sweep = true;
         }
      } else if (input.has_range) {
         res.sweep = true;
      }
      if (input.raycast.has_value()) {
         const auto& raycast = input.raycast.value();
         if (raycast.hit_position.has_value()) {
            res.hit_position = raycast.hit_position.value();
            res.target       = raycast.target_info.form;
         }
      }
      all_responses.merge_member(input, res);
   }
   /*static*/ void attempt_on_screen_selection::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }
}

#include "../../core.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void attempt_on_screen_selection::invoke(const response& params) {
      auto& worldedit_core = core::get();

      if (params.sweep) {
         //
         // TODO
         //
         #if !_DEBUG
            static_assert(false, "TODO: Only modify an entity's selection state on the first frame the cursor sweeps over it.");
         #endif
      }
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