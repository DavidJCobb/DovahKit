#pragma once
#include "./_base.h"
#include "dovah/forms/Container.h"
#include "ui_container.h" // generated

class FormDialogContainer :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Container, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogContainer(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogContainer ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
