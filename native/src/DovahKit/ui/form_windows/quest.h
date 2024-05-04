#pragma once
#include "./_base.h"
#include "dovah/forms/Quest.h"
#include "ui_quest.h"

class QuestTabStages;
class QuestTabObjectives;

class FormDialogQuest :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Quest, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogQuest(dovah::form_stub& stub, QWidget* parent = Q_NULLPTR);
      
   protected:
      Ui::FormDialogQuest ui;
      struct {
         QuestTabStages*     stages     = nullptr;
         QuestTabObjectives* objectives = nullptr;
      } tabs;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
