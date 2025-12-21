#pragma once
#include "./_base.h"
#include "dovah/forms/ImagespaceModifier.h"
#include "ui_imagespace_modifier.h" // generated

#include "ui/types/imagespace_modifier/keyframe_collection.h"

class FormDialogImagespaceModifier :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ImagespaceModifier, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogImagespaceModifier(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogImagespaceModifier ui;
      ui::types::imagespace_modifier::keyframe_collection keyframes;
      float last_position = 0.0F;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _set_current_position(float position);

      void _load_keyframe(float position);
      void _save_keyframe(float position);
};
