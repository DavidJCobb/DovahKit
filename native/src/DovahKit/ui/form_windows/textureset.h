#pragma once
#include "_base.h"
#include "../../dovah/forms/TextureSet.h"
#include "ui_textureset.h"

class FormDialogTextureSet : public FormDialogBaseTemplate {
   Q_OBJECT
   DOVAHKIT_FORM_EDIT_DIALOG
   public:
      FormDialogTextureSet(dovah::form_stub* stub, QWidget* parent = Q_NULLPTR);
      
   public slots:
      void refreshTextureList();
      
   protected:
      Ui::FormDialogTextureSet ui;
      dovah::loaded_form_ptr<dovah::loaded_forms::TextureSet> form;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
