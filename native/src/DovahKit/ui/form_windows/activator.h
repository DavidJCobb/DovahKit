#pragma once
#include "./_base.h"
#include "dovah/forms/Activator.h"
#include "ui_activator.h"

class DKPapyrusBoundScriptListPane;

class FormDialogActivator : public FormWorkingCopyEditDialogBase {
   Q_OBJECT
   DOVAHKIT_FORM_COPY_EDIT_DIALOG(dovah::loaded_forms::Activator)
   public:
      FormDialogActivator(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      
   protected:
      Ui::FormDialogActivator ui;
      struct {
         DKPapyrusBoundScriptListPane* papyrus = nullptr;
      } subwidgets;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
