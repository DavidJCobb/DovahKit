#pragma once
#include "./_base.h"
#include "dovah/forms/ImpactData.h"
#include "ui_impact_data.h" // generated

class FormDialogImpactData :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ImpactData, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogImpactData(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogImpactData ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
