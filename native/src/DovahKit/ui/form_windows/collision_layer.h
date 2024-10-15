#pragma once
#include "./_base.h"
#include "dovah/forms/CollisionLayer.h"
#include "ui_collision_layer.h" // generated

class FormDialogCollisionLayer :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::CollisionLayer, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogCollisionLayer(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogCollisionLayer ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
