#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::form_types {
   struct imagespace_modifier_keyframe_interp : debug_feature {
      static constexpr const char* name = "ImageSpaceModifier keyframe interpolation";
      static void execute(QWidget* from);
   };
}