#pragma once
#include "./_base.h"
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
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      QModelIndex _selected_qmi() const noexcept;
      void _pull_selected_node_to_ui();
      void _push_selected_node_from_ui();
};
