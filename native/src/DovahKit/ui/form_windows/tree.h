#pragma once
#include "./_base.h"
#include "dovah/forms/Tree.h"
#include "ui_tree.h"

class FormDialogTree :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Tree, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogTree(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogTree ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
