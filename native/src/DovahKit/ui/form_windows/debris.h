#pragma once
#include "./_base.h"
#include "dovah/forms/Debris.h"
#include "ui_debris.h"
class DebrisVariantsModel;

class FormDialogDebris :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Debris, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogDebris(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogDebris ui;
      struct {
         DebrisVariantsModel* variants = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
