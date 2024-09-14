#pragma once
#include "./_base.h"
#include "dovah/forms/DialogueBranch.h"
#include "ui_dialogue_branch.h" // generated

class DialogueBranchStartingTopicFormFilter;

class FormDialogDialogueBranch :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::DialogueBranch, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogDialogueBranch(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogDialogueBranch ui;
      struct {
         DialogueBranchStartingTopicFormFilter* starting_topic = nullptr;
      } _filters;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
