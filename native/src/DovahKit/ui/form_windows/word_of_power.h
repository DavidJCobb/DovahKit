#pragma once
#include "./_base.h"
#include "dovah/forms/WordOfPower.h"
#include "ui_word_of_power.h"

class FormDialogWordOfPower :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::WordOfPower, false>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogWordOfPower(dovah::form_stub& stub, QWidget* parent = nullptr);

   protected:
      Ui::FormDialogWordOfPower ui;

      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
