#pragma once
#include "./_base.h"
#include "dovah/forms/Worldspace.h"
#include "ui_worldspace.h"
class DKFormPickerExcludeSingleFormFilter;

class FormDialogWorldspace :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Worldspace, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogWorldspace(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogWorldspace ui;
      struct {
         DKFormPickerExcludeSingleFormFilter* exclude_self = nullptr;
      } filters;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _on_has_parent_changed(bool force = false);
};
