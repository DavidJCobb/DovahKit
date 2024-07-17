#pragma once
#include "./_base.h"
#include "dovah/forms/Global.h"
#include "ui_global.h" // generated

class FormDialogGlobal :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Global, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogGlobal(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogGlobal ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
