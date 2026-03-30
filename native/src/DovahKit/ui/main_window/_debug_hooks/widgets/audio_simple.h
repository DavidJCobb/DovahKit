#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct audio_simple : debug_feature {
      static constexpr const char* name = "DKAudioWidgetSimple";
      static void execute(QWidget* from);
   };
}