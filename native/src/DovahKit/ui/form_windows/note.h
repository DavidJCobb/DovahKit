#pragma once
#include "./_base.h"
#include "dovah/forms/Note.h"
#include "ui_note.h" // generated

class FormDialogNote :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Note, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogNote(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogNote ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
