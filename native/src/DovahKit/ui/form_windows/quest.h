#pragma once
#include "./_base.h"
#include "dovah/forms/Quest.h"
#include "ui_quest.h"

class QuestTabStages;
class QuestTabObjectives;

class FormDialogQuest : public FormWorkingCopyEditDialogBase {
   Q_OBJECT
   DOVAHKIT_FORM_COPY_EDIT_DIALOG(dovah::loaded_forms::Quest)
   public:
      FormDialogQuest(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      
   private slots:
      
   protected:
      Ui::FormDialogQuest ui;
      struct {
         QuestTabStages*     stages     = nullptr;
         QuestTabObjectives* objectives = nullptr;
      } tabs;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
