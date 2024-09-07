#pragma once
#include "./_base.h"
#include "dovah/forms/SoulGem.h"
#include "ui_soul_gem.h" // generated

class FormDialogSoulGem :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::SoulGem, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogSoulGem(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogSoulGem ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
