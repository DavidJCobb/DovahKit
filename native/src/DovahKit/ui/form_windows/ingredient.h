#pragma once
#include "./_base.h"
#include "dovah/forms/Ingredient.h"
#include "ui_ingredient.h" // generated

class FormDialogIngredient :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Ingredient, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogIngredient(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogIngredient ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _update_auto_calc();
};
