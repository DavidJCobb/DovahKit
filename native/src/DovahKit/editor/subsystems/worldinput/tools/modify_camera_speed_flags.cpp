#include "modify_camera_speed_flags.h"
#include "./_options.h"
#include "./_results.h"
#include "../input_result.h"

namespace dovahkit::subsystems::worldinput::tools {
   void modify_camera_speed_flags::results::merge(const results& from) {
      if (from.boost != bool_operation::no_op)
         this->boost = from.boost;
      if (from.precision != bool_operation::no_op)
         this->precision = from.precision;
   }

   void modify_camera_speed_flags::invoke(const input_result& input, const opaque_option_union& raw_options, combined_tool_results& all_results) const {
      const auto* o = option_union::as<options>(raw_options);
      if (!o)
         return;
      //
      // TODO: Handle ReferenceFrames here, or provide an option for them in DKVulkanCameraUpdate. 
      // The former approach requires access to camera state from here; the latter approach does 
      // not, but would require us to reset the movement vector if the update state already has a 
      // different reference frame (e.g. if there are multiple inconsistent move_camera keybinds). 
      // 
      // Currently, we always treat movement as camera-relative (comments on DKVulkanCameraUpdate 
      // saying it's world-relative are currently wrong).
      //
      results res = {
         .boost     = o->boost,
         .precision = o->precision,
      };
      if (input.active()) {
         switch (input.type) {
            using _ = control_type;
            case _::button:
               if (input.button.press_type == button_press_type::while_down) {
                  if (!input.button.changed) {
                     //
                     // If false, the key is still down; would fire every tick. "Invert" 
                     // writes should be canceled so that we're not constantly toggling 
                     // a flag's state every tick while the button is down.
                     //
                     if (res.boost == bool_operation::invert)
                        res.boost = bool_operation::no_op;
                     if (res.precision == bool_operation::invert)
                        res.precision = bool_operation::no_op;
                  }
               }
               break;
            case _::scalar:
            case _::vector:
               //
               // "Invert" mappings can't work here.
               //
               if (res.boost == bool_operation::invert)
                  res.boost = bool_operation::no_op;
               if (res.precision == bool_operation::invert)
                  res.precision = bool_operation::no_op;
               break;
         }
      } else {
         //
         // Input is not still active.
         //
         if (input.is_button_release()) {
            //
            // A "while-down" bind was released.
            //
            auto invert = [](bool_operation& b) {
               switch (b) {
                  using enum bool_operation;
                  case set_true:  b = set_false; break;
                  case set_false: b = set_true;  break;
               }
            };
            invert(res.boost);
            invert(res.precision);
         }
      }
      all_results.merge_member(input, res);
   }
}