#pragma once
#include "./_base.h"
#include "dovah/forms/Key.h"

// Keys subclass MiscItems in the game engine, and the UI and data are identical.
#include "ui_misc_item.h" // generated

class FormDialogKey :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Key, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogKey(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMiscItem ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
