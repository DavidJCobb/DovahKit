#include "debug_log.h"
#include "_options.h"
#include "../InputResult.h"

namespace DK3D::tools {
   void debug_log::invoke(const InputResult& input, const opaque_option_union& raw_options, DKVulkanCameraUpdate& camera_update) const {
      const auto* o = option_union::as<options>(raw_options);
      if (!o)
         return;
      if (!input.active()) {
         if (input.while_has_changed) {
            qDebug("[DK3D::tools::debug_log::invoke] A \"While\" bind has been released. Param was %d.", o->number);
         }
         return;
      }
      switch (input.type) {
         using _ = InputResult::Type;
         case _::Boolean:
            switch (input.bool_mod) {
               using _ = BooleanInputMod;
               case _::Tap:
                  qDebug("[DK3D::tools::debug_log::invoke] A \"Tap\" bind has been pressed and released. Param was %d.", o->number);
                  break;
               case _::Hold:
                  qDebug("[DK3D::tools::debug_log::invoke] A \"Hold\" bind has been pressed and released. Param was %d.", o->number);
                  break;
               case _::While:
                  if (input.while_has_changed)
                     //
                     // If false, the key is still down; would fire every tick.
                     //
                     qDebug("[DK3D::tools::debug_log::invoke] A \"While\" bind has been pressed. Param was %d.", o->number);
                  break;
            }
            break;
         case _::Scalar:
            //qDebug("[DK3D::tools::debug_log::invoke] \"Scalar\" bind invoked with input %f. Param was %d.", input.x, o->number);
            break;
         case _::Vector:
            //qDebug("[DK3D::tools::debug_log::invoke] \"Vector\" bind invoked with input (%f, %f). Param was %d.", input.x, input.y, o->number);
            break;
      }
   }
}