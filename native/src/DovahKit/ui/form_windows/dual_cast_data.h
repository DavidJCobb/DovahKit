#pragma once
#include "./_base.h"
#include "dovah/forms/DualCastData.h"
#include "ui_dual_cast_data.h" // generated

class FormDialogDualCastData :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::DualCastData, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogDualCastData(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogDualCastData ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
