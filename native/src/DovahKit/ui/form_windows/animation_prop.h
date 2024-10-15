#pragma once
#include "./_base.h"
#include "dovah/forms/AnimationProp.h"
#include "ui_animation_prop.h" // generated

class FormDialogAnimationProp :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::AnimationProp, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogAnimationProp(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogAnimationProp ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
