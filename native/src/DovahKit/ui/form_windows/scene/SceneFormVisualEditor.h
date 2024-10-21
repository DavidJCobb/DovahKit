#pragma once
#include <cstdint>
#include <QWidget>
#include "dovah/form_stub.h"
#include "./SceneFormVisualEditor_impl/ActorBehaviorFlag.h"
#include "./SceneFormVisualEditor_impl/Style.h"
#include "ui/types/conditions/condition.h"

namespace dovah {
   namespace loaded_forms {
      class Quest;
      class Scene;
   }
   class form_stub;
}
namespace SceneFormVisualEditor_impl {
   class Action;
   class Actor;
   class Phase;
}
class QuestAllDialogueDatastore;

class SceneFormVisualEditor : public QWidget {
   Q_OBJECT;
   public:
      SceneFormVisualEditor(QWidget* parent = nullptr);
      ~SceneFormVisualEditor();

   public:
      struct SceneContext {
         dovah::form_stub*           quest    = nullptr;
         dovah::form_stub*           scene    = nullptr;
         QuestAllDialogueDatastore*  dialogue = nullptr;
      };

   public:
      void setContext(const SceneContext&);

      constexpr dovah::form_stub* scene() const noexcept { return this->_context.scene; }

      bool isReferenceAliasUsed(uint32_t id) const;
      QString referenceAliasName(uint32_t id) const;

      // import/export backend data for the current context scene's working copy
      void importData();
      void exportData();

      void clear();
      void spawnRenderTest();

      void popBehaviorFlagDialog();
      void popParticipationFlagDialog();
      
   protected:
      using Style  = SceneFormVisualEditor_impl::Style;
      using Actor  = SceneFormVisualEditor_impl::Actor;
      using Phase  = SceneFormVisualEditor_impl::Phase;
      using Action = SceneFormVisualEditor_impl::Action;

      void removeActor(uint32_t alias_id);

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
      SceneContext _context;
      struct {
         std::vector<Actor*>  actors;
         std::vector<Phase*>  phases;
         std::vector<Action*> actions;
      } _data;
      Style _style;

      ui::types::conditions::context _make_condition_context() const;
      void _update_phase_conditions(bool trigger_geometry_update = true);
      void _update_cached_internal_relationships();
      void _update_geometry();
};