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
class ActorBaseTintLayerModel;
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
      Ui::FormDialogActorBase ui;
      struct {
         QMenu relationships;
      } _context_menus;
      struct {
         impl::CrimeFactionPickerFilter* crime_faction = nullptr;
         impl::DKFormPickerExcludeSingleFormFilter* exclude_self = nullptr;
         struct {
            HeadPartPickerFilter*             base_head_part = nullptr;
            impl::FaceComplexionPickerFilter* complexion     = nullptr;
            impl::FaceHairColorPickerFilter*  hair_color     = nullptr;
         } face;
         impl::VoicetypePickerFilter* voicetype = nullptr;
      } _filters;
      struct {
         ActorBaseCreatureSoundsModel* creature_sounds  = nullptr;
         ActorBaseTintLayerModel*      face_tints       = nullptr;
         ActorBaseFactionsModel*       factions         = nullptr;
         FaceBaseHeadPartsModel*       head_parts_base  = nullptr;
         FaceExtraHeadPartsModel*      head_parts_extra = nullptr;
         ActorBaseRelationshipsModel*  relationships    = nullptr;
         ActorBaseSkillsModel*         skills           = nullptr;
      } _models;

      // Singleton signal handlers:
      void _on_game_setting_changed(const char* name);
      void _on_other_form_modified(dovah::form_stub*);
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _update_outfit_contents_view();

      dovah::form_stub* _is_templated_from_leveled_base() const; // returns Leveled Character if so
      void _set_template_actor(dovah::form_stub*);
      void _update_from_template_actor();
      void _push_data_to_ui(loaded_form_type::template_flag::type);

      unsigned int _get_effective_level() const;
      void _on_effective_level_changed();

      void _recalc_stats();

      void _pull_faction_to_ui();
      void _push_faction_from_ui();

      void _set_creature_sound_inherit_actor(dovah::form_stub* stub); // NOTE: If we can't legally inherit from `stub`, makes no change but still updates UI.
      void _creature_sound_inheritance_changed();
      void _pull_creature_sound_to_ui();
      void _push_creature_sound_from_ui();

      void _pull_tint_layer_to_ui();
      void _push_tint_layer_from_ui();

      dovah::sex _current_sex() const;

      void _set_pc_level_mult(bool);
      void _set_race(dovah::form_stub*);
      void _set_sex(dovah::sex);
};
