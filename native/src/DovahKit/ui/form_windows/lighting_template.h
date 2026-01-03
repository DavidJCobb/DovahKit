#pragma once
#include "./_base.h"
#include "dovah/forms/LightingTemplate.h"
#include "ui_lighting_template.h"

class FormDialogLightingTemplate :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::LightingTemplate, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogLightingTemplate(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogLightingTemplate ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _copy_from_cell();
      void _pull_dalc_to_ui();
};
