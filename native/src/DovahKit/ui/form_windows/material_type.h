#pragma once
#include "./_base.h"
#include "dovah/forms/MaterialType.h"
#include "ui_material_type.h" // generated

class DKFormPickerExcludeSingleFormFilter;

class FormDialogMaterialType :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::MaterialType, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMaterialType(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMaterialType ui;
      struct {
         DKFormPickerExcludeSingleFormFilter* exclude_self = nullptr;
      } _filters;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
