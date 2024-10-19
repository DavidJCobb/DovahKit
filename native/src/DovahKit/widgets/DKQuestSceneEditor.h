#pragma once
#include <cstdint>
#include <QWidget>
#include "dovah/data/dialogue/emotion.h"
#include "./widget-subtypes/DKQuestSceneEditor/ActorBehaviorFlag.h"
#include "./widget-subtypes/DKQuestSceneEditor/Style.h"

namespace dovah {
   class form_stub;
}
namespace DKQuestSceneEditor_impl {
   class Action;
   class Actor;
   class Phase;
}

class DKQuestSceneEditor : public QWidget {
   Q_OBJECT;
   public:
      DKQuestSceneEditor(QWidget* parent = nullptr);
      ~DKQuestSceneEditor();

      void clear();
      void spawnRenderTest();
      
   protected:
      using Style  = DKQuestSceneEditor_impl::Style;
      using Actor  = DKQuestSceneEditor_impl::Actor;
      using Phase  = DKQuestSceneEditor_impl::Phase;
      using Action = DKQuestSceneEditor_impl::Action;

   public:
      #pragma region Overrides
         virtual QSize minimumSizeHint() const override;
         virtual QSize sizeHint() const override;

         virtual void paintEvent(QPaintEvent* event) override;
      #pragma endregion

   protected:
      struct {
         QSize size;
      } _cached;
      struct {
         std::vector<Actor*>  actors;
         std::vector<Phase*>  phases;
         std::vector<Action*> actions;
      } _data;
      Style _style;

      void _update_cached_internal_relationships();
      void _update_geometry();
};