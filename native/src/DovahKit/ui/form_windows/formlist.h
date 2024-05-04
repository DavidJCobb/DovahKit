#pragma once
#include "./_base.h"
#include "dovah/forms/FormList.h"
#include "ui_formlist.h"

class FormDialogFormList :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::FormList, false>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogFormList(dovah::form_stub& stub, QWidget* parent = Q_NULLPTR);
      
   protected:
      Ui::FormDialogFormList ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
