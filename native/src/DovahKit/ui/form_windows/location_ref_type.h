#pragma once
#include <cstdint>
#include <QDialog>
#include "./_base.h"
#include "dovah/forms/LocationRefType.h"

// LocRefTypes subclass Keywords in the game engine, and the UI and data are identical.
#include "ui_keyword.h" // generated

class FormDialogLocationRefType :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::LocationRefType, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogLocationRefType(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogKeyword ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
