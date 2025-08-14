#pragma once
#include "./_base.h"
#include "dovah/forms/Book.h"
#include "ui_book.h"

class FormDialogBook :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Book, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogBook(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogBook ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
