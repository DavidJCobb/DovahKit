#pragma once
#include "./_base.h"
#include "dovah/forms/ActorValueInfo.h"
#include "ui_actor_value_info.h"

class FormDialogActorValueInfo :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ActorValueInfo, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogActorValueInfo(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogActorValueInfo ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
