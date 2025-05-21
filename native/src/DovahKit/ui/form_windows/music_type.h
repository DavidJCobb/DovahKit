#pragma once
#include "./_base.h"
#include "dovah/forms/MusicType.h"
#include "ui_music_type.h" // generated

class FormDialogMusicType :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::MusicType, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMusicType(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMusicType ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
