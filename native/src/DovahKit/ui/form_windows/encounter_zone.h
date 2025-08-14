#pragma once
#include "./_base.h"
#include "dovah/forms/EncounterZone.h"
#include "dovah/forms/Faction.h"
#include "ui_encounter_zone.h" // generated

class FormDialogEncounterZone :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::EncounterZone, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogEncounterZone(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogEncounterZone ui;
      struct {
         dovah::form_stub* form = nullptr;
         int32_t           rank = 0;
         dovah::loaded_form_ptr<dovah::loaded_forms::Faction> loaded_faction;
      } working_ownership;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _update_ownership_widgets();
      void _update_rank_picker();
};
