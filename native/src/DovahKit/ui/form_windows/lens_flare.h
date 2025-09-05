#pragma once
#include "./_base.h"
#include "dovah/forms/LensFlare.h"
#include "ui_lens_flare.h"

class LensFlareSpritesModel;

class FormDialogLensFlare :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::LensFlare, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogLensFlare(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogLensFlare ui;
      struct {
         LensFlareSpritesModel* sprites = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
