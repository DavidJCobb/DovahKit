#pragma once
#include "./_base.h"
#include "dovah/forms/MovementType.h"
#include "ui_movement_type.h" // generated

class FormDialogMovementType :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::MovementType, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMovementType(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMovementType ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
