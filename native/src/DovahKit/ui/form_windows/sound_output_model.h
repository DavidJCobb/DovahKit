#pragma once
#include "./_base.h"
#include "dovah/forms/SoundOutputModel.h"
#include "ui_sound_output_model.h" // generated

class FormDialogSoundOutputModel :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::SoundOutputModel, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogSoundOutputModel(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogSoundOutputModel ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _pull_channel_to_ui();
      void _push_channel_from_ui();
};
