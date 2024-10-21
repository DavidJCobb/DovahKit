#pragma once
#include <vector>
#include "dovah/data/dialogue/emotion.h"
#include "./Action.h"

namespace dovah {
   class form_stub;
}

namespace SceneFormVisualEditor_impl {
   class DialogueAction : public Action {
      public:
         virtual void paint(QPainter&, const Style&) override;
         virtual void recalcSize(int width, const Style&, const QFontMetrics&) override;

      public:
         dovah::form_stub* topic = nullptr;
         uint32_t headtrack_alias_id = -1;
         struct {
            dovah::dialogue::emotion type = dovah::dialogue::emotion::neutral;
            uint32_t value = 50;
         } emotion;
         struct {
            float min = 0.0F;
            float max = 0.0F;
         } looping;

         struct {
            std::vector<QString> infos;
         } cached;
         struct {
            std::vector<int> infos;
         } body_geometry;
   };
}