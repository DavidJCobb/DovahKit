#pragma once
#include "./_base.h"
#include "dovah/forms/Apparatus.h"
#include "ui_apparatus.h"

class FormDialogApparatus :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Apparatus, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogApparatus(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogApparatus ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
