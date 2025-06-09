#pragma once
#include "./_base.h"
#include "dovah/forms/Enchantment.h"
#include "ui_enchantment.h" // generated

class DKFormPickerExcludeSingleFormFilter;

class FormDialogEnchantment :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Enchantment, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogEnchantment(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogEnchantment ui;
      struct {
         DKFormPickerExcludeSingleFormFilter* exclude_self = nullptr;
      } _filters;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _update_auto_calc();
      void _update_effect_parameters_enable_states();
};
