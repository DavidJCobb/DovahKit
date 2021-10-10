#pragma once
#include "_base.h"
#include "../../dovah/forms/LandTexture.h"
#include "ui_landtexture.h"

class FormDialogLandTexture : public FormDialogBaseTemplate {
   Q_OBJECT
   DOVAHKIT_FORM_EDIT_DIALOG
   public:
      FormDialogLandTexture(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      //
   private slots:
      //
   protected:
      Ui::FormDialogLandTexture ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::LandTexture> form;
      //
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
