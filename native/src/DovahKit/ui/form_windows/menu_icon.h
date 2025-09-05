#pragma once
#include "./_base.h"
#include "dovah/forms/MenuIcon.h"
#include "ui_menu_icon.h"

class FormDialogMenuIcon :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::MenuIcon, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMenuIcon(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMenuIcon ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
