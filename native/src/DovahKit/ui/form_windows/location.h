#pragma once
#include "./_base.h"
#include "dovah/forms/Location.h"
#include "ui_location.h" // generated
class DKFormPickerExcludeSingleFormFilter;

class FormDialogLocation :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Location, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogLocation(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogLocation ui;
      struct {
         DKFormPickerExcludeSingleFormFilter* exclude_self = nullptr;
      } _filters;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _update_contents_views();
};
