#pragma once
#include "./_base.h"
#include "dovah/forms/Explosion.h"
#include "ui_explosion.h" // generated

class FormDialogExplosion :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Explosion, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogExplosion(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogExplosion ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
