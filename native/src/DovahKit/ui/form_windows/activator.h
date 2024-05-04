#pragma once
#include "./_base.h"
#include "dovah/forms/Activator.h"
#include "ui_activator.h"

class FormDialogActivator :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Activator, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogActivator(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogActivator ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
