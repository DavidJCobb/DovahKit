#pragma once
#include <QDialog>
#include "./_base.h"
#include "dovah/forms/Voicetype.h"
#include "ui_voicetype.h"

class FormDialogVoicetype :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Voicetype, false>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogVoicetype(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogVoicetype ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
