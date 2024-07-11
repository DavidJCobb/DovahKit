#pragma once
#include "./_base.h"
#include "dovah/forms/Light.h"
#include "ui_light.h" // generated

class FormDialogLight :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Light, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogLight(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogLight ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
