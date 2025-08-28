#pragma once
#include "./_base.h"
#include "dovah/forms/IdleMarker.h"
#include "ui_idle_marker.h"

class FormDialogIdleMarker :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::IdleMarker, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogIdleMarker(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogIdleMarker ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
