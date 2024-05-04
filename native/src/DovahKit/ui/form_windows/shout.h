#pragma once
#include "./_base.h"
#include "dovah/forms/Shout.h"
#include "ui_shout.h"

class FormDialogShout :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Shout, false>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogShout(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogShout ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
