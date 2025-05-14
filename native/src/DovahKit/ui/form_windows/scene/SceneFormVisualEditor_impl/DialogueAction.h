#pragma once
#include <vector>
#include "dovah/data/dialogue/emotion.h"
#include "./Action.h"

namespace dovah {
   class form_stub;
}

namespace SceneFormVisualEditor_impl {
   struct DialogueActionData {
      struct {
         dovah::dialogue::emotion type = dovah::dialogue::emotion::neutral;
         uint32_t value = 50;
      } emotion;
      struct {
         uint32_t alias_id    = -1;
         bool     at_player   = false;
         bool     face_target = false;
      } headtrack;
      struct {
         bool  enabled = false;
         float min     = 0.0F;
         float max     = 0.0F;
      } looping;
      dovah::form_stub* topic = nullptr;
   };

   class DialogueAction : public Action {
      public:
         virtual ActionType type() const noexcept override { return ActionType::Dialogue; };
         virtual void paint(QPainter&, const Style&, const StyleOption&) override;
         virtual void recalcSize(int width, const Style&, const QFontMetrics&) override;

      public:
         DialogueActionData data;

         struct {
            std::vector<QString> infos;
         } cached;
         struct {
            std::vector<int> infos; // heights
         } body_geometry;
   };
}