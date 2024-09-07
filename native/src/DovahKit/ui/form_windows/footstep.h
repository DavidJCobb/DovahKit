#pragma once
#include "./_base.h"
#include "dovah/forms/Footstep.h"
#include "ui_footstep.h" // generated

class FormDialogFootstep :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Footstep, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogFootstep(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogFootstep ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
