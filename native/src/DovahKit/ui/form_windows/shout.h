#pragma once
#include "_base.h"
#include "../../dovah/forms/Shout.h"
#include "ui_shout.h"

class FormDialogShout : public FormDialogBaseTemplate {
   Q_OBJECT
   DOVAHKIT_FORM_EDIT_DIALOG
   public:
      FormDialogShout(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::FormDialogShout ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::Shout> form;
      //
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
