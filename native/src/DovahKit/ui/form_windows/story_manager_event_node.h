#pragma once
#include "./_base.h"
#include <QMenu>
#include "dovah/forms/StoryManagerEventNode.h"
#include "ui_story_manager_event_node.h" // generated

class FormDialogStoryManagerNodes :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::StoryManagerEventNode, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogStoryManagerNodes(dovah::form_stub& stub, QWidget* parent = nullptr);

      void focusForm(const dovah::form_stub&);

   protected:
      using loaded_node_base_type = dovah::loaded_forms::mixins::StoryManagerNode;
      
   protected:
      Ui::FormDialogStoryManagerNodes ui;
      struct {
         QMenu menu;
         struct {
            struct {
               struct {
                  QAction* new_branch_node  = nullptr;
                  QAction* new_quest_node   = nullptr;
               } branch;
               struct {
                  QAction* add_quests = nullptr;
               } quest;
               QAction* delete_node = nullptr;
            } node;
            struct {
               QAction* edit   = nullptr;
               QAction* remove = nullptr;
            } quest_form;
            QAction* use_info = nullptr;
         } actions;
      } context;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      QModelIndex _selected_qmi() const noexcept; // mapped from our proxy model to the SM subsystem model
      std::pair<QModelIndex, dovah::form_stub*> _selected_source_model_item() const noexcept;
      void _pull_selected_node_to_ui();
      void _push_selected_node_from_ui();

      void _focus_qmi(const QModelIndex&);

      #pragma region Context menu
         void _context_branch_new_child_branch();
         void _context_branch_new_child_quest_list();
         void _context_quest_list_add_quests();
         void _context_delete_node();
         void _context_quest_form_edit();
         void _context_quest_form_remove();
         void _context_use_info();
      #pragma endregion
};
