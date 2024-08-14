#pragma once
#include "./_base.h"
#include "dovah/forms/ActorBase.h"
#include "ui_actor_base.h" // generated

#include <QMenu>
#include "dovah/data/sex.h"

namespace impl {
   class CrimeFactionPickerFilter;
   class DKFormPickerExcludeSingleFormFilter;
   class FaceComplexionPickerFilter;
   class FaceHairColorPickerFilter;
   class FaceTintColorPickerFilter;
   class VoicetypePickerFilter;
}
class ActorBaseCreatureSoundsModel;
class ActorBaseFaceTintsModel;//static_assert(false, "TODO: Implement me!");
class ActorBaseFactionsModel;
class ActorBaseRelationshipsModel;
class ActorBaseSkillsModel;
class FaceBaseHeadPartsModel;
class FaceExtraHeadPartsModel;
class HeadPartPickerFilter;

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
      struct HeadData {
         dovah::form_stub* complexion = nullptr;
         dovah::form_stub* hair_color = nullptr;
         struct {
            ActorBaseFaceTintsModel* tints            = nullptr;
            FaceBaseHeadPartsModel*  base_head_parts  = nullptr;
            FaceExtraHeadPartsModel* extra_head_parts = nullptr;
         } models;
      };

      Ui::FormDialogActorBase ui;
      struct {
         QMenu relationships;
      } _context_menus;
      struct {
         //
         // Maintain two sets of head data so that if the player switches an actor's sex 
         // back and forth, they don't lose (as much) data.
         //
         HeadData female;
         HeadData male;
      } _data;
      struct {
         impl::CrimeFactionPickerFilter* crime_faction = nullptr;
         impl::DKFormPickerExcludeSingleFormFilter* exclude_self = nullptr;
         struct {
            HeadPartPickerFilter*             base_head_part = nullptr;
            impl::FaceComplexionPickerFilter* complexion     = nullptr;
            impl::FaceHairColorPickerFilter*  hair_color     = nullptr;
            impl::FaceTintColorPickerFilter*  tint_color     = nullptr;
         } face;
         impl::VoicetypePickerFilter* voicetype = nullptr;
      } _filters;
      struct {
         ActorBaseCreatureSoundsModel* creature_sounds = nullptr;
         ActorBaseFactionsModel*       factions        = nullptr;
         ActorBaseRelationshipsModel*  relationships   = nullptr;
         ActorBaseSkillsModel*         skills          = nullptr;
      } _models;
      struct {
         // If we detect that the current Template Actor would form a cyclical reference 
         // (including if changes are made to other actors which cause this), then we want 
         // to warn the user. However, if our window doesn't have focus, then we should 
         // delay the warning.
         struct {
            bool warned = false;
            dovah::form_stub* our_template = nullptr;
            dovah::form_stub* seen_twice   = nullptr;
         } pending_cyclical_template_actor_warn;
      } _state;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
      virtual void event(QEvent*) override;

      void _update_outfit_contents_view();
      void _update_from_template_actor();
      void _show_cyclical_template_actor_warning();
      void _push_data_to_ui(loaded_form_type::template_flag::type);

      unsigned int _get_effective_level() const;
      void _on_effective_level_changed();

      void _recalc_stats();

      void _pull_faction_to_ui();
      void _push_faction_from_ui();

      void _creature_sound_inheritance_changed();
      void _pull_creature_sound_to_ui();
      void _push_creature_sound_from_ui();

      dovah::sex _current_sex() const;

      void _set_pc_level_mult(bool);
      void _set_race(dovah::form_stub*);
      void _set_sex(dovah::sex);
};
