#pragma once
#include "./_base.h"
#include "dovah/forms/MiscItem.h"
#include "ui_misc_item.h" // generated

class FormDialogMiscItem :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::MiscItem, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMiscItem(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMiscItem ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
