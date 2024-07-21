#pragma once
#include "./_base.h"
#include "dovah/forms/AssociationType.h"
#include "ui_association_type.h" // generated

class FormDialogAssociationType :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::AssociationType, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogAssociationType(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogAssociationType ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
