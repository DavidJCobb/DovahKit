#include "debug_placeholder.h"
#include "_options.h"
#include "../input_result.h"

namespace DK3D::tools {
   void debug_placeholder::invoke(const input_result& input, const opaque_option_union& raw_options, combined_tool_results&) const {
      const auto* o = option_union::as<options>(raw_options);
      if (!o)
         return;
      if (!input.active()) {
         if (input.is_button_release()) {
            qDebug("[DK3D::tools::debug_placeholder::invoke] A \"While\" bind has been released. Param was %s.", qUtf8Printable(o->text));
         }
         return;
      }
      switch (input.type) {
         using _ = control_type;
         case _::button:
            switch (input.button.press_type) {
               using _ = button_press_type;
               case _::tap:
                  qDebug("[DK3D::tools::debug_placeholder::invoke] A \"Tap\" bind has been pressed and released. Param was %s.", qUtf8Printable(o->text));
                  break;
               case _::hold:
                  qDebug("[DK3D::tools::debug_placeholder::invoke] A \"Hold\" bind has been pressed and released. Param was %s.", qUtf8Printable(o->text));
                  break;
               case _::while_down:
                  if (input.button.changed)
                     //
                     // If false, the key is still down; would fire every tick.
                     //
                     qDebug("[DK3D::tools::debug_placeholder::invoke] A \"While\" bind has been pressed. Param was %s.", qUtf8Printable(o->text));
                  break;
            }
            break;
         case _::scalar:
            //qDebug("[DK3D::tools::debug_log::invoke] \"Scalar\" bind invoked with input %f. Param was %s.", input.x, qUtf8Printable(o->text));
            break;
         case _::vector:
            //qDebug("[DK3D::tools::debug_log::invoke] \"Vector\" bind invoked with input (%f, %f). Param was %s.", input.x, input.y, qUtf8Printable(o->text));
            break;
      }
   }
}