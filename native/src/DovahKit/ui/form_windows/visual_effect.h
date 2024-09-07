#pragma once
#include "./_base.h"
#include "dovah/forms/VisualEffect.h"
#include "ui_visual_effect.h" // generated

class FormDialogVisualEffect :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::VisualEffect, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogVisualEffect(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogVisualEffect ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
