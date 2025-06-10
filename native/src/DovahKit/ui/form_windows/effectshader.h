#pragma once
#include "./_base.h"
#include "dovah/forms/EffectShader.h"
#include "ui_effectshader.h" // generated

class FormDialogEffectShader :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::EffectShader, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogEffectShader(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogEffectShader ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
