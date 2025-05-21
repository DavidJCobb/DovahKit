#pragma once
#include "./_base.h"
#include "dovah/forms/Sound.h"
#include "ui_sound.h" // generated

class FormDialogSound :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Sound, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogSound(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogSound ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
