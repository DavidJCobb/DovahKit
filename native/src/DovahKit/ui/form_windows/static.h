#pragma once
#include "./_base.h"
#include "dovah/forms/Static.h"
#include "ui_static.h" // generated

class FormDialogStatic :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Static, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogStatic(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogStatic ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
