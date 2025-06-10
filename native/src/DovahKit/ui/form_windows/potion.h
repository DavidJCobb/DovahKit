#pragma once
#include "./_base.h"
#include "dovah/forms/Potion.h"
#include "ui_potion.h" // generated

class FormDialogPotion :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Potion, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogPotion(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogPotion ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _update_auto_calc();
};
