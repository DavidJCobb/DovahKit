#pragma once
#include "./_base.h"
#include "dovah/forms/Door.h"
#include "ui_door.h" // generated

class FormDialogDoor :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Door, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogDoor(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogDoor ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
