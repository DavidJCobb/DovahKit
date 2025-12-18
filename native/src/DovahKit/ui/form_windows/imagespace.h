#pragma once
#include "./_base.h"
#include "dovah/forms/Imagespace.h"
#include "ui_imagespace.h" // generated

class FormDialogImagespace :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Imagespace, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogImagespace(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogImagespace ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
