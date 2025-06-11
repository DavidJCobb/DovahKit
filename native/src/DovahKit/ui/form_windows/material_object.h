#pragma once
#include "./_base.h"
#include "dovah/forms/MaterialObject.h"
#include "ui_material_object.h" // generated

class FormDialogMaterialObject :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::MaterialObject, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMaterialObject(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMaterialObject ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
