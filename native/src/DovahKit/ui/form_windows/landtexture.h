#pragma once
#include "./_base.h"
#include "dovah/forms/LandTexture.h"
#include "ui_landtexture.h"

class FormDialogLandTexture :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::LandTexture, false>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogLandTexture(dovah::form_stub& stub, QWidget* parent = Q_NULLPTR);
      
   private slots:
      
   protected:
      Ui::FormDialogLandTexture ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
