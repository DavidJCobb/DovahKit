#include "debug_dump_landscape_details.h"
#include <QCursor>
#include "./_options.h"
#include "./_results.h"
#include "../input_result.h"

namespace dovahkit::subsystems::worldinput::tools {
   void debug_dump_landscape_details::invoke(const input_result& input, const opaque_option_union& raw_options, combined_tool_results& all_results) const {
      if (!input.active())
         return;
      const auto* o = option_union::as<options>(raw_options);
      if (!o)
         return;
      //
      results res = {
         .position  = o->position,
         .mouse     = { input.x, input.y },
      };
      switch (input.type) {
         using _ = control_type;
         case _::button:
            if (input.button.press_type == button_press_type::while_down) {
               if (!input.button.changed) {
                  break;
               }
            }
            res.exists = true;
            break;
         case _::scalar:
         case _::vector:
            //
            // TODO: How do we pass a mouse position here?
            //
            if (o->position == pointer_position_type::mouse) {
               res.exists = true;
               res.mouse  = QCursor::pos(); // HACK HACK HACK
            }
            break;
      }
      all_results.merge_member(input, res);
   }
}