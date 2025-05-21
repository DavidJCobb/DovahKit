#pragma once
#include "./_base.h"
#include "dovah/forms/CameraShot.h"
#include "ui_camera_shot.h" // generated

class FormDialogCameraShot :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::CameraShot, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogCameraShot(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogCameraShot ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
