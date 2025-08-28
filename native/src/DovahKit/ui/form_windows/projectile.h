#pragma once
#include "./_base.h"
#include "dovah/forms/Projectile.h"
#include "ui_projectile.h"

class FormDialogProjectile :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Projectile, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogProjectile(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogProjectile ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
