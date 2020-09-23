#pragma once
#include "_base.h"
#include "../../dovah/forms/WordOfPower.h"
#include "ui_word_of_power.h"

class FormDialogWordOfPower : public FormDialogBaseTemplate {
   Q_OBJECT
   DOVAHKIT_FORM_EDIT_DIALOG
   public:
      FormDialogWordOfPower(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::FormDialogWordOfPower ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::WordOfPower> form;
      //
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
