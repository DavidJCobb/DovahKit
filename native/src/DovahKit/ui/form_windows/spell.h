#pragma once
#include "./_base.h"
#include "dovah/forms/Spell.h"
#include "ui_spell.h" // generated

class FormDialogSpell :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Spell, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogSpell(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogSpell ui;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      QString _make_concatenated_description() const;
      void _update_auto_calc();
      void _update_condition_explanation();
      void _update_effect_parameters_enable_states(dovah::form_stub* changed = nullptr);
};
