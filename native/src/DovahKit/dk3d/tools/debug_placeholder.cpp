#include "debug_placeholder.h"
#include "_options.h"
#include "../InputResult.h"

namespace DK3D::tools {
   void debug_placeholder::invoke(const InputResult& input, const opaque_option_union& raw_options, DKVulkanCameraUpdate& camera_update) const {
      const auto* o = option_union::as<options>(raw_options);
      if (!o)
         return;
      if (!input.active()) {
         if (input.while_has_changed) {
            qDebug("[DK3D::tools::debug_placeholder::invoke] A \"While\" bind has been released. Param was %s.", qUtf8Printable(o->text));
         }
         return;
      }
      switch (input.type) {
         using _ = InputResult::Type;
         case _::Boolean:
            switch (input.bool_mod) {
               using _ = BooleanInputMod;
               case _::Tap:
                  qDebug("[DK3D::tools::debug_placeholder::invoke] A \"Tap\" bind has been pressed and released. Param was %s.", qUtf8Printable(o->text));
                  break;
               case _::Hold:
                  qDebug("[DK3D::tools::debug_placeholder::invoke] A \"Hold\" bind has been pressed and released. Param was %s.", qUtf8Printable(o->text));
                  break;
               case _::While:
                  if (input.while_has_changed)
                     //
                     // If false, the key is still down; would fire every tick.
                     //
                     qDebug("[DK3D::tools::debug_placeholder::invoke] A \"While\" bind has been pressed. Param was %s.", qUtf8Printable(o->text));
                  break;
            }
            break;
         case _::Scalar:
            //qDebug("[DK3D::tools::debug_log::invoke] \"Scalar\" bind invoked with input %f. Param was %s.", input.x, qUtf8Printable(o->text));
            break;
         case _::Vector:
            //qDebug("[DK3D::tools::debug_log::invoke] \"Vector\" bind invoked with input (%f, %f). Param was %s.", input.x, input.y, qUtf8Printable(o->text));
            break;
      }
   }
}