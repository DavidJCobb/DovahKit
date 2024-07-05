#pragma once
#include "./_base.h"
#include "dovah/forms/Flora.h"
#include "ui_flora.h"

class FormDialogFlora :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Flora, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogFlora(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogFlora ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
