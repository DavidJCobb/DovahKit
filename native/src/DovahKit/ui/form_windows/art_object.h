#pragma once
#include "./_base.h"
#include "dovah/forms/ArtObject.h"
#include "ui_art_object.h" // generated

class FormDialogArtObject :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ArtObject, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogArtObject(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogArtObject ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
