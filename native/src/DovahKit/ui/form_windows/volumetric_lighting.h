#pragma once
#include "./_base.h"
#include "dovah/forms/VolumetricLighting.h"
#include "ui_volumetric_lighting.h"

class FormDialogVolumetricLighting :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::VolumetricLighting, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogVolumetricLighting(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogVolumetricLighting ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
