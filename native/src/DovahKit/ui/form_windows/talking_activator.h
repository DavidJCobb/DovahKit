#pragma once
#include "./_base.h"
#include "dovah/forms/TalkingActivator.h"
#include "ui_talking_activator.h"

class FormDialogTalkingActivator :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::TalkingActivator, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogTalkingActivator(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogTalkingActivator ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
