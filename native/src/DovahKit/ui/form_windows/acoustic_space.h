#pragma once
#include "./_base.h"
#include "dovah/forms/AcousticSpace.h"
#include "ui_acoustic_space.h" // generated

class FormDialogAcousticSpace :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::AcousticSpace, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogAcousticSpace(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogAcousticSpace ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
