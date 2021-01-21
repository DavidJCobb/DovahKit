#pragma once
#include "_base.h"
#include "../../dovah/forms/Quest.h"
#include "ui_quest.h"

class FormDialogQuest : public FormDialogWorkingCopyBase {
   Q_OBJECT
   DOVAHKIT_FORM_EDIT_DIALOG
   public:
      using form_t = dovah::loaded_forms::Quest;
   public:
      FormDialogQuest(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::FormDialogQuest ui;
      //
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
