#pragma once
#include "_base.h"
#include "../../dovah/forms/FormList.h"
#include "ui_formlist.h"

class FormDialogFormList : public FormDialogBaseTemplate {
   Q_OBJECT
   DOVAHKIT_FORM_EDIT_DIALOG
   public:
      FormDialogFormList(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::FormDialogFormList ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::FormList> form;
      //
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
