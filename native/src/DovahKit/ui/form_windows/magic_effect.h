#pragma once
#include "./_base.h"
#include <array>
#include "dovah/forms/MagicEffect.h"
#include "ui_magic_effect.h" // generated

class DKFormPickerExcludeSingleFormFilter;
class MagicEffectSummonableActorPickerFilter;

class FormDialogMagicEffect :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::MagicEffect, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMagicEffect(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMagicEffect ui;
      struct {
         DKFormPickerExcludeSingleFormFilter*    exclude_self      = nullptr;
         MagicEffectSummonableActorPickerFilter* summonable_actors = nullptr;
      } _filters;
      struct {
         struct {
            dovah::form_stub* draw_sheathe = nullptr;
            dovah::form_stub* charge = nullptr;
            dovah::form_stub* ready = nullptr;
            dovah::form_stub* release = nullptr;
            dovah::form_stub* concentration_cast_loop = nullptr;
            dovah::form_stub* on_hit = nullptr;
         } sounds;
      } _state;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      struct associated_item_constraint {
         dovah::form_type  form_type = dovah::form_type::none;
         dovah::form_stub* forced_av = nullptr;
         bool actors_must_be_summonable = false;
      };
      std::array<associated_item_constraint, 2> constraints_for_archetype() const;

      void on_archetype_changed(bool initial_load = false);
      void on_associated_items_changed();
};
