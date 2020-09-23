#pragma once
#include <cstdint>
#include <QDialog>
#include "_base.h"
#include "../../dovah/forms/Voicetype.h"
#include "ui_voicetype.h"

class FormDialogVoicetype : public FormDialogBaseTemplate {
   Q_OBJECT
   DOVAHKIT_FORM_EDIT_DIALOG
   public:
      FormDialogVoicetype(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::FormDialogVoicetype ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::Voicetype> form;
      //
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
