#pragma once
#include "./_base.h"
#include "dovah/forms/HeadPart.h"
#include "ui_head_part.h" // generated

class FormDialogHeadPart :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::HeadPart, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogHeadPart(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogHeadPart ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
