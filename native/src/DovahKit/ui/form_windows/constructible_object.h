#pragma once
#include "./_base.h"
#include "dovah/forms/ConstructibleObject.h"
#include "ui_constructible_object.h"

class FormDialogConstructibleObject :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ConstructibleObject, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogConstructibleObject(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogConstructibleObject ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
