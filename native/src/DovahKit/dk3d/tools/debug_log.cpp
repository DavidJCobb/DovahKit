#include "debug_log.h"
#include "_options.h"
#include "../input_result.h"

namespace DK3D::tools {
   void debug_log::invoke(const input_result& input, const opaque_option_union& raw_options, combined_tool_results&) const {
      const auto* o = option_union::as<options>(raw_options);
      if (!o)
         return;
      if (!input.active()) {
         if (input.is_button_release()) {
            qDebug("[DK3D::tools::debug_log::invoke] A \"While\" bind has been released. Param was %d.", o->number);
         }
         return;
      }
      switch (input.type) {
         using _ = control_type;
         case _::button:
            switch (input.button.press_type) {
               using _ = button_press_type;
               case _::tap:
                  qDebug("[DK3D::tools::debug_log::invoke] A \"Tap\" bind has been pressed and released. Param was %d.", o->number);
                  break;
               case _::hold:
                  qDebug("[DK3D::tools::debug_log::invoke] A \"Hold\" bind has been pressed and released. Param was %d.", o->number);
                  break;
               case _::while_down:
                  if (input.button.changed)
                     //
                     // If false, the key is still down; would fire every tick.
                     //
                     qDebug("[DK3D::tools::debug_log::invoke] A \"While\" bind has been pressed. Param was %d.", o->number);
                  break;
            }
            break;
         case _::scalar:
            //qDebug("[DK3D::tools::debug_log::invoke] \"Scalar\" bind invoked with input %f. Param was %d.", input.x, o->number);
            break;
         case _::vector:
            //qDebug("[DK3D::tools::debug_log::invoke] \"Vector\" bind invoked with input (%f, %f). Param was %d.", input.x, input.y, o->number);
            break;
      }
   }
}