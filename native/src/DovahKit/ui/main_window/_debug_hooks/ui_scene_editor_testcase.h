#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct ui_scene_editor_testcase : debug_feature {
      static constexpr const char* name = "Test DKQuestSceneEditor";
      static void execute(QWidget* from);
   };
}