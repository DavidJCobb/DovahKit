#pragma once
#include "./Action.h"

namespace dovah {
   class form_stub;
}

namespace SceneFormVisualEditor_impl {
   class TimerAction : public Action {
      public:
         virtual void paint(QPainter&, const Style&) override;
         virtual void recalcSize(int width, const Style&, const QFontMetrics&) override;

      public:
         float duration = 0.0F; // seconds
   };
}