#pragma once
#include "./_base.h"
#include "dovah/forms/SoundCategory.h"
#include "ui_sound_category.h" // generated

class DKFormPickerExcludeSingleFormFilter;

class FormDialogSoundCategory :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::SoundCategory, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogSoundCategory(dovah::form_stub& stub, QWidget* parent = nullptr);

      // When the Sound Category dialog's "Categorize Sounds" button is used, 
      // any recategorized categtories that have open dialogs should have the 
      // dialogs update.
      void forceRefreshParentCategory();
      
   protected:
      Ui::FormDialogSoundCategory ui;
      struct {
         DKFormPickerExcludeSingleFormFilter* exclude_self  = nullptr;
      } _filters;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
