#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include <QMenu>
#include <QPointer>
#include <QScrollArea>
#include <QWidget>
#include "dovah/form_stub.h"
#include "./SceneFormVisualEditor_impl/ActorBehaviorFlag.h"
#include "./SceneFormVisualEditor_impl/ActionType.h"
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

      // HACK. Ideally, the QScrollArea should be part of this widget itself.
      void setContainingScrollArea(QScrollArea*);

      constexpr bool hasAnyChanges() const noexcept { return this->_data.any_changes_made; }

      constexpr float zoom() const noexcept { return this->_zoom; }
      void setZoom(float);
      
   protected:
      using Style  = SceneFormVisualEditor_impl::Style;
      using Actor  = SceneFormVisualEditor_impl::Actor;
      using Phase  = SceneFormVisualEditor_impl::Phase;
      using Action = SceneFormVisualEditor_impl::Action;

      using ActionType = SceneFormVisualEditor_impl::ActionType;

      // Map a point from global to chart-relative, i.e. accounting for the current zoom.
      QPoint _map_from_global(const QPoint&);

      struct MouseTargetAreaDetails {
         bool exact  : 1 = false;
         bool edge_l : 1 = false;
         bool edge_r : 1 = false;
         bool edge_t : 1 = false;
         bool edge_b : 1 = false;
      };
      //
      template<typename T>
      struct MouseTargetArea : MouseTargetAreaDetails {
         T* pointer = nullptr;
      };
      //
      struct MouseTargets {
         MouseTargetArea<Action> action;
         MouseTargetArea<Actor>  actor;
         MouseTargetArea<Phase>  phase;
      };
      //
      MouseTargets _find_mouse_targets(const QPoint& local_pos);

      struct DragDropTarget {
         Actor* actor = nullptr;
         Phase* phase = nullptr;
         size_t phase_index = 0;
      };
      DragDropTarget _find_drag_drop_target(QRect dragged_rect, ActionType dragged_action_type, uint32_t dragged_action_id); // rect is widget-relative
      DragDropTarget _find_drag_drop_target(const QPoint& widget_relative_pos, ActionType dragged_action_type, uint32_t dragged_action_id);

      void _on_resize_ended();

      void _set_up_action_dialog_phases(const Action&, FormSubdialogSceneActionBase&);

      void _update_cursor(const QMouseEvent*);

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
         virtual void mouseMoveEvent(QMouseEvent*) override;
         virtual void mouseReleaseEvent(QMouseEvent*) override;
         #pragma region Drag and drop
            virtual void dragEnterEvent(QDragEnterEvent*) override;
            virtual void dragMoveEvent(QDragMoveEvent*) override;
            virtual void dropEvent(QDropEvent*) override;
         #pragma endregion
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
      struct {
         QPoint                  mousedown_at; // widget-relative
         MouseTargetArea<Action> mousedown_on;
         QPoint                  mouse_prev_pos; // screen-relative
         bool                    is_panning   = false;
         bool                    is_resizing  = false;
      } _mouse;
      QPointer<QScrollArea> _scroll_area;
      std::variant<
         std::monostate,
         Phase*,
         Action*
      > _selection;
      Style _style;
      float _zoom = 1.0F;
      
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