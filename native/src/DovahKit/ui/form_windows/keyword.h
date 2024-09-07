#pragma once
#include <cstdint>
#include <QDialog>
#include "./_base.h"
#include "dovah/forms/Keyword.h"
#include "ui_keyword.h"

class FormDialogKeyword :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Keyword, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogKeyword(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogKeyword ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
