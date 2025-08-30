#pragma once
#include "./_base.h"
#include "dovah/forms/WaterType.h"
#include "ui_water_type.h"

class FormDialogWaterType :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::WaterType, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogWaterType(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogWaterType ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
