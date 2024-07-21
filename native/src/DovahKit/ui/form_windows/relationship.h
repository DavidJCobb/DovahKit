#pragma once
#include "./_base.h"
#include "dovah/forms/Relationship.h"
#include "ui_relationship.h" // generated

class FormDialogRelationship :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Relationship, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogRelationship(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogRelationship ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
