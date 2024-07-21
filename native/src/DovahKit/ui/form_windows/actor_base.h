#pragma once
#include "./_base.h"
#include "dovah/forms/ActorBase.h"
#include "ui_actor_base.h" // generated

namespace impl {
   class DKFormPickerExcludeSingleFormFilter;
   class VoicetypePickerFilter;
}
class FormDialogActorBase :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ActorBase, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogActorBase(dovah::form_stub& stub, QWidget* parent = nullptr);

   public slots:
      void updatePreview();
      
   protected:
      Ui::FormDialogActorBase ui;
      struct {
         impl::DKFormPickerExcludeSingleFormFilter* exclude_self = nullptr;
         impl::VoicetypePickerFilter* voicetype = nullptr;
      } _filters;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _updateOutfitContentsView();
      void _updateFromTemplate();
};
