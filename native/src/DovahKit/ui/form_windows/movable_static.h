#pragma once
#include "./_base.h"
#include "dovah/forms/MovableStatic.h"
#include "ui_movable_static.h"

class FormDialogMovableStatic :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::MovableStatic, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMovableStatic(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMovableStatic ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
