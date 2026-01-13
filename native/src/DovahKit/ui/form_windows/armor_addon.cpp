#include "./armor_addon.h"
#include "widgets/DKFormNIFPicker.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "./armor_addon/ArmorAddonAdditionalRacesModel.h"
#include "./shared/BipedObjectSlotsToggleModel.h"

FormDialogArmorAddon::FormDialogArmorAddon(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::set_range<float>(this->ui.weaponAdjust);
   ui::set_range<decltype(loaded_form_type::addon_data::priority)>(this->ui.priorityM);
   ui::set_range<decltype(loaded_form_type::addon_data::priority)>(this->ui.priorityF);
   this->ui.race->setAllowedFormType(dovah::form_type::race);
   this->ui.artObject->setAllowedFormType(dovah::form_type::art_object);
   this->ui.skinTexM->setAllowedFormType(dovah::form_type::texture_set);
   this->ui.skinTexF->setAllowedFormType(dovah::form_type::texture_set);
   this->ui.skinSwapListM->setAllowedFormType(dovah::form_type::formlist);
   this->ui.skinSwapListF->setAllowedFormType(dovah::form_type::formlist);
   this->ui.footstepSet->setAllowedFormType(dovah::form_type::footstep_set);

   {
      auto* model = new BipedObjectSlotsToggleModel(this);
      this->_models.biped_objects = model;
      auto* widget = this->ui.bipedObject;
      widget->setModel(model);
   }
   QObject::connect(this->ui.race, &DKFormPicker::formChanged, this, [this](dovah::form_stub* race) {
      if (race)
         this->_models.biped_objects->setSlotNamesFrom(*race);
   });
   this->ui.race->setAllowNone(false);

   {
      auto* model = new ArmorAddonAdditionalRacesModel(this);
      this->_models.additional_races = model;
      auto* widget = this->ui.additionalRaces;
      widget->setModel(model);
   }

   this->load(); // this creates the working copy.
}
void FormDialogArmorAddon::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.weaponAdjust, working.weapon_adjust);
   ui::bind(this->ui.race, working.races.primary, working);
   ui::bind(this->ui.artObject, working.art_object, working);

   {
      auto& src = working.graphics[dovah::sex::male];
      this->ui.bipedModelM->initializeFrom(src.models.third_person);
      ui::bind(this->ui.skinTexM,      src.skin_texture.base,      working);
      ui::bind(this->ui.skinSwapListM, src.skin_texture.swap_list, working);
      this->ui.fpModelM->initializeFrom(src.models.first_person);
      ui::bind(this->ui.priorityM, src.priority);
   }
   {
      auto& src = working.graphics[dovah::sex::female];
      this->ui.bipedModelF->initializeFrom(src.models.third_person);
      ui::bind(this->ui.skinTexF,      src.skin_texture.base,      working);
      ui::bind(this->ui.skinSwapListF, src.skin_texture.swap_list, working);
      this->ui.fpModelF->initializeFrom(src.models.first_person);
      ui::bind(this->ui.priorityF, src.priority);
   }

   ui::bind(this->ui.footstepSet, working.footstep_sound, working);
   ui::bind(this->ui.detectSoundValue, working.loudness);

   this->_models.biped_objects->importFlags(working.biped_object);
   this->_models.additional_races->initializeFrom(working.races.additional);
}
void FormDialogArmorAddon::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   {
      auto& dst = working.graphics[dovah::sex::male];
      this->ui.bipedModelM->commitTo(dst.models.third_person, working);
      this->ui.fpModelM->commitTo(dst.models.first_person, working);
      {
         auto* widget   = this->ui.bipedModelM;
         auto  path     = std::filesystem::path(widget->value().model_path);
         auto  filename = path.filename().string();
         if (filename.ends_with("_0") || filename.ends_with("_1")) {
            dst.flags |= loaded_form_type::addon_flag::enable_weight_slider;
         } else {
            dst.flags &= ~loaded_form_type::addon_flag::enable_weight_slider;
         }
      }
   }
   {
      auto& dst = working.graphics[dovah::sex::female];
      this->ui.bipedModelF->commitTo(dst.models.third_person, working);
      this->ui.fpModelF->commitTo(dst.models.first_person, working);
      {
         auto* widget   = this->ui.bipedModelF;
         auto  path     = std::filesystem::path(widget->value().model_path);
         auto  filename = path.filename().string();
         if (filename.ends_with("_0") || filename.ends_with("_1")) {
            dst.flags |= loaded_form_type::addon_flag::enable_weight_slider;
         } else {
            dst.flags &= ~loaded_form_type::addon_flag::enable_weight_slider;
         }
      }
   }

   this->_models.biped_objects->exportFlags(working.biped_object);
   this->_models.additional_races->commitTo(working.races.additional, working);
}