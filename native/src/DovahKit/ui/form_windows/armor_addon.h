#pragma once
#include "./_base.h"
#include "dovah/forms/ArmorAddon.h"
#include "ui_armor_addon.h"

class ArmorAddonAdditionalRacesModel;
class BipedObjectSlotsToggleModel;

class FormDialogArmorAddon :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ArmorAddon, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogArmorAddon(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogArmorAddon ui;
      struct {
         ArmorAddonAdditionalRacesModel* additional_races = nullptr;
         BipedObjectSlotsToggleModel*    biped_objects    = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
