#pragma once
#include "./_base.h"
#include "dovah/forms/ReverbParameters.h"
#include "ui_reverb_parameters.h" // generated

class FormDialogReverbParameters :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ReverbParameters, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogReverbParameters(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogReverbParameters ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
