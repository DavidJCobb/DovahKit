#pragma once
#include "./_base.h"
#include "dovah/forms/Climate.h"
#include "ui_climate.h"

class ClimateWeathersModel;

class FormDialogClimate :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Climate, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogClimate(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogClimate ui;
      struct {
         ClimateWeathersModel* weathers = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
