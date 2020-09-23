#pragma once
#include <cstdint>
#include <QDialog>
#include "_base.h"
#include "../../dovah/forms/Color.h"
#include "ui_color.h"

class FormDialogColor : public FormDialogBaseTemplate {
   Q_OBJECT
   DOVAHKIT_FORM_EDIT_DIALOG
   public:
      FormDialogColor(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::FormDialogColor ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::Color> form;
      //
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
