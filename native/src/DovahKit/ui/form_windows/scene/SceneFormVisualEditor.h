#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include <QMenu>
#include <QWidget>
#include "dovah/form_stub.h"
#include "./SceneFormVisualEditor_impl/ActorBehaviorFlag.h"
#include "./SceneFormVisualEditor_impl/SceneContext.h"
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
class FormSubdialogSceneActionBase;
class QuestAllDialogueDatastore;

class SceneFormVisualEditor : public QWidget {
   Q_OBJECT;
   public:
      SceneFormVisualEditor(QWidget* parent = nullptr);
      ~SceneFormVisualEditor();

   public:
      using SceneContext = SceneFormVisualEditor_impl::SceneContext;

   public:
      void setContext(const SceneContext&);

      constexpr dovah::form_stub* scene() const noexcept { return this->_context.scene; }

      bool isReferenceAliasUsed(uint32_t id) const;
      QString referenceAliasName(uint32_t id) const;

      // import/export backend data for the current context scene's working copy
      void importData();
      void exportData();

      void focus_dialogue_forms(uint32_t action_id, dovah::form_stub* topic, dovah::form_stub* info);

   protected:
      void _clear_data();
   public:
      void clear();
      void spawnRenderTest();

      void popBehaviorFlagDialog();
      void popParticipationFlagDialog();

      constexpr bool hasAnyChanges() const noexcept { return this->_data.any_changes_made; }
      
   protected:
      using Style  = SceneFormVisualEditor_impl::Style;
      using Actor  = SceneFormVisualEditor_impl::Actor;
      using Phase  = SceneFormVisualEditor_impl::Phase;
      using Action = SceneFormVisualEditor_impl::Action;

      void _set_up_action_dialog_phases(const Action&, FormSubdialogSceneActionBase&);

      void addActor();
      void editAction(Action&);
      void editPhase(Phase&);
      void insertAction(Action&, Actor&, Phase&);
      void insertPhaseAt(size_t);
      void removeActor(uint32_t alias_id);
      void removeAction(Action&);
      void removePhase(Phase&);

   public:
      #pragma region Overrides
         virtual QSize minimumSizeHint() const override;
         virtual QSize sizeHint() const override;

         virtual void contextMenuEvent(QContextMenuEvent* event) override;
         virtual void mouseDoubleClickEvent(QMouseEvent*) override;
         virtual void mousePressEvent(QMouseEvent*) override;
         virtual void paintEvent(QPaintEvent*) override;
      #pragma endregion

   protected:
      struct {
         QSize size;
      } _cached;
      SceneContext _context;
      struct {
         QMenu menu;

         QAction* new_actor  = nullptr;
         QMenu new_action;
         struct {
            QAction* dialogue = nullptr;
            QAction* package  = nullptr;
            QAction* timer    = nullptr;
         } new_action_type;
         QMenu new_phase;
         struct {
            QAction* before_here = nullptr;
            QAction* after_here = nullptr;
            QAction* at_end = nullptr;
         } new_phase_where;
         QAction* edit   = nullptr;
         QAction* remove = nullptr;
      } _context_menu;
      struct {
         std::vector<Actor*>  actors;
         std::vector<Phase*>  phases;
         std::vector<Action*> actions;
         bool any_changes_made = false;
      } _data;
      std::variant<
         std::monostate,
         Phase*,
         Action*
      > _selection;
      Style _style;
      
      void _deselect_all();
      void _select(Action*);
      void _select(Phase*);

      constexpr Action* _selected_action() const {
         if (std::holds_alternative<Action*>(this->_selection))
            return std::get<Action*>(this->_selection);
         return nullptr;
      }
      constexpr Phase* _selected_phase() const {
         if (std::holds_alternative<Phase*>(this->_selection))
            return std::get<Phase*>(this->_selection);
         return nullptr;
      }

      ui::types::conditions::context _make_condition_context() const;
      void _update_phase_conditions(bool trigger_geometry_update = true);
      void _update_cached_internal_relationships();
      void _update_geometry();
};