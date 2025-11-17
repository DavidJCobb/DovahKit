#pragma once
#include <QAction>
#include <QDialog>
#include <QMenu>
#include "ui_idle.h" // generated
#include "dovah/forms/IdleAnimation.h"
#include "dovah/form_stub.h"
namespace dovah::exceptions {
   class form_creation_failed;
}

class IdleAnimationsDialog : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type = dovah::loaded_forms::IdleAnimation;

   public:
      IdleAnimationsDialog(QWidget* parent = nullptr);

      void focusIdle(dovah::form_stub&);

   protected:
      Ui::IdleAnimationsDialog ui;
      struct {
         QMenu* menu = nullptr;
         struct {
            struct {
               QAction* create          = nullptr;
               QAction* add_action_root = nullptr;
            } graph;
            struct {
               QAction* use_info = nullptr;
            } action;
            struct {
               QAction* create    = nullptr;
               QMenu*   duplicate = nullptr;
               QAction* del       = nullptr;
               QAction* use_info  = nullptr;
            } idle;
         } actions;
      } _context;
      dovah::loaded_form_ptr<loaded_form_type> _current_idle;

      QModelIndex _get_selected_row();

      #pragma region Idle tree context menu
         void _context_add_action_root();
         void _context_add_graph();
         void _context_add_idle();
         void _context_duplicate_idle_single();
         void _context_duplicate_idle_tree();
         void _context_delete_idle();
         void _context_use_info();
      #pragma endregion

      void _report_idle_create_error(const dovah::exceptions::form_creation_failed&);

      void _pull_selected_idle_to_ui();
      void _push_selected_idle_to_form();
      void _set_form_ui_enable_state(bool);
};