#pragma once
#include <cstdint>
#include <QDialog>
#include "./_base.h"
#include "dovah/forms/ActorAction.h"
#include "ui_actor_action.h"

class FormDialogActorAction :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ActorAction, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogActorAction(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogActorAction ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
