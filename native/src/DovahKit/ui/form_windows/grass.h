#pragma once
#include "./_base.h"
#include "dovah/forms/Grass.h"
#include "ui_grass.h" // generated

class FormDialogGrass :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Grass, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogGrass(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogGrass ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
