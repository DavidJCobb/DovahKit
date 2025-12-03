#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct scene_editor : debug_feature {
      static constexpr const char* name = "DKQuestSceneEditor";
      static void execute(QWidget* from);
   };
}