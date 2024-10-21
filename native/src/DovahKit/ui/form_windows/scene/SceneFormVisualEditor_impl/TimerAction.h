#pragma once
#include "./Action.h"
#include "./ScriptFragment.h"

namespace dovah {
   class form_stub;
}

namespace SceneFormVisualEditor_impl {
   struct TimerActionData {
      float duration = 0.0F; // seconds
      ScriptFragment fragment;
   };

   class TimerAction : public Action {
      public:
         virtual void paint(QPainter&, const Style&, const StyleOption&) override;
         virtual void recalcSize(int width, const Style&, const QFontMetrics&) override;

      public:
         TimerActionData data;
   };
}