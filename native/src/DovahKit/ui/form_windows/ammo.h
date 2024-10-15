#pragma once
#include "./_base.h"
#include "dovah/forms/Ammo.h"
#include "ui_ammo.h" // generated

class FormDialogAmmo :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Ammo, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogAmmo(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogAmmo ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
