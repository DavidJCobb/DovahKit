#pragma once
#include "./_base.h"
#include "dovah/forms/ActorBase.h"
#include "ui_actor_base.h" // generated

#include <QMenu>

namespace impl {
   class CrimeFactionPickerFilter;
   class DKFormPickerExcludeSingleFormFilter;
   class VoicetypePickerFilter;
}
class ActorBaseFactionsModel;
class ActorBaseRelationshipsModel;
class ActorBaseSkillsModel;

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
         impl::VoicetypePickerFilter* voicetype = nullptr;
      } _filters;
      struct {
         ActorBaseFactionsModel*      factions      = nullptr;
         ActorBaseRelationshipsModel* relationships = nullptr;
         ActorBaseSkillsModel*        skills        = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _updateOutfitContentsView();
      void _updateFromTemplate();

      void _pull_faction_to_ui();
      void _push_faction_from_ui();
};
