#pragma once
#include "./_base.h"
#include "dovah/forms/LoadingScreen.h"
#include "ui_loading_screen.h"

class FormDialogLoadingScreen :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::LoadingScreen, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogLoadingScreen(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogLoadingScreen ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
