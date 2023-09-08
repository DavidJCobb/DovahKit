#include "./modify_camera_speed_flags.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   void modify_camera_speed_flags::response::merge(const response& from) {
      if (from.boost != bool_operation::no_op)
         this->boost = from.boost;
      if (from.precision != bool_operation::no_op)
         this->precision = from.precision;
   }

   /*static*/ void modify_camera_speed_flags::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const auto& o = raw_options.as<options>();
      //
      // TODO: Handle ReferenceFrames here, or provide an option for them in DKVulkanCameraUpdate. 
      // The former approach requires access to camera state from here; the latter approach does 
      // not, but would require us to reset the movement vector if the update state already has a 
      // different reference frame (e.g. if there are multiple inconsistent move_camera keybinds). 
      // 
      // Currently, we always treat movement as camera-relative (comments on DKVulkanCameraUpdate 
      // saying it's world-relative are currently wrong).
      //
      response res = {
         .boost     = o.boost,
         .precision = o.precision,
      };
      if (input.has_button) {
         if (!input.button.down_state_changed_this_frame) {
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
      } else {
         //
         // "Invert" mappings can't work here.
         //
         if (res.boost == bool_operation::invert)
            res.boost = bool_operation::no_op;
         if (res.precision == bool_operation::invert)
            res.precision = bool_operation::no_op;
      }
      all_results.merge_member(input, res);
   }
   /*static*/ void modify_camera_speed_flags::request_for_hold_release(const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();
      //
      response res = {
         .boost     = o.boost,
         .precision = o.precision,
      };
      {
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
      all_results.merge_member(res);
   }
}