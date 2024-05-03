#pragma once
#include <cstdint>
#include <QDialog>
#include "./_base.h"
#include "dovah/forms/Voicetype.h"
#include "ui_voicetype.h"

class FormDialogVoicetype : public FormEditDialogBase {
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG(dovah::loaded_forms::Voicetype);
   public:
      FormDialogVoicetype(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      
   protected:
      Ui::FormDialogVoicetype ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
