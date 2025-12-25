#pragma once
#include "./_base.h"
#include "dovah/forms/ImpactDataSet.h"
#include "ui_impact_data_set.h" // generated

class ImpactDataSetContentsModel;

class FormDialogImpactDataSet :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ImpactDataSet, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogImpactDataSet(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogImpactDataSet ui;
      ImpactDataSetContentsModel* _model = nullptr;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
