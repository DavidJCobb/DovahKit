#pragma once
#include "./_base.h"
#include "dovah/forms/Armor.h"
#include "ui_armor.h"

class BipedObjectSlotsToggleModel;
class DKFormPickerExcludeSingleFormFilter;

class FormDialogArmor :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Armor, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogArmor(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogArmor ui;
      struct {
         DKFormPickerExcludeSingleFormFilter* exclude_self = nullptr;
      } _filters;
      struct {
         BipedObjectSlotsToggleModel* biped_objects = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
