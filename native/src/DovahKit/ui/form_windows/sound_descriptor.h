#pragma once
#include "./_base.h"
#include "dovah/forms/SoundDescriptor.h"
#include "ui_sound_descriptor.h" // generated

class FormDialogSoundDescriptor :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::SoundDescriptor, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogSoundDescriptor(dovah::form_stub& stub, QWidget* parent = nullptr);

      // When the Sound Category dialog's "Categorize Sounds" button is used, 
      // any recategorized descriptors that have open dialogs should have the 
      // dialogs update.
      void forceRefreshParentCategory();
      
   protected:
      Ui::FormDialogSoundDescriptor ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
