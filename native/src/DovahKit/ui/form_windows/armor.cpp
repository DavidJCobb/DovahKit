#include "./armor.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "widgets/DKFormNIFPicker.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "./shared/BipedObjectSlotsToggleModel.h"
#include "./shared/DKFormPickerExcludeSingleFormFilter.h"

FormDialogArmor::FormDialogArmor(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->_filters.exclude_self = new DKFormPickerExcludeSingleFormFilter(this);
   {
      auto* model = new BipedObjectSlotsToggleModel(this);
      this->_models.biped_objects = model;
      auto* widget = this->ui.bipedObjectsCurrentList;
      widget->setModel(model);
   }

   ui::set_unsigned_range<decltype(loaded_form_type::value)>(this->ui.value);
   this->ui.enchantment->setAllowedFormType(dovah::form_type::enchantment);
   this->ui.templateForm->setAllowedFormType(loaded_form_type::form_type);
   ui::set_unsigned_range<decltype(loaded_form_type::weight)>(this->ui.weight);
   ui::set_unsigned_range<decltype(loaded_form_type::rating)>(this->ui.rating);
   {
      using enumeration = decltype(dovah::loaded_forms::components::biped_object::armor_type);
      auto* widget = this->ui.armorSkill;
      widget->clear();
      widget->addItem(tr("Light Armor"), (int)enumeration::light_armor);
      widget->addItem(tr("Heavy Armor"), (int)enumeration::heavy_armor);
      widget->addItem(tr("Clothing"),    (int)enumeration::clothing);
   }
   this->ui.equipType->setAllowedFormType(dovah::form_type::equip_slot);
   this->ui.impactDataSetBlockBash->setAllowedFormType(dovah::form_type::impact_data_set);
   this->ui.alternateBlockMaterial->setAllowedFormType(dovah::form_type::material_type);
   this->ui.bipedObjectsCurrentRace->setAllowedFormType(dovah::form_type::race);
   this->ui.soundTake->setAllowedFormTypes({ dovah::form_type::sound_descriptor });
   this->ui.soundDrop->setAllowedFormTypes({ dovah::form_type::sound_descriptor });
   this->ui.armorAddons->setAllowedFormTypes({ dovah::form_type::armor_addon });
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });

   QObject::connect(this->ui.bipedObjectsCurrentRace, &DKFormPicker::formChanged, this, [this](dovah::form_stub* race) {
      if (race)
         this->_models.biped_objects->setSlotNamesFrom(*race);
   });
   this->ui.bipedObjectsCurrentRace->setAllowNone(false);

   this->load(); // this creates the working copy.
}
void FormDialogArmor::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   this->_filters.exclude_self->set_exclusion(this->formStub());

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   ui::bind(this->ui.value, working.value);
   ui::bind(this->ui.enchantment, working.enchantable.effect, working);
   ui::bind(this->ui.templateForm, working.template_armor, working);
   ui::bind(this->ui.weight, working.weight);
   ui::bind(this->ui.armorSkill, working.biped_object.armor_type);
   ui::bind(this->ui.rating, working.rating);
   ui::bind_inverse(this->ui.flagPlayable, record_flags(), loaded_form_type::form_flag::non_playable);
   ui::bind(this->ui.equipType, working.equip_type, working);
   ui::bind(this->ui.impactDataSetBlockBash, working.block_bash.impact_data_set, working);
   ui::bind(this->ui.alternateBlockMaterial, working.block_bash.alternate_material, working);
   this->ui.description->setPlainText(gls.convert_localized_string(working.description));
   ui::bind(this->ui.bipedObjectsCurrentRace, working.race, working);
   this->ui.modelM->initializeFrom(working.world_models[dovah::sex::male].model);
   ui::bind(this->ui.iconInventoryM, working.world_models[dovah::sex::male].icons.inventory);
   ui::bind(this->ui.iconMessageM, working.world_models[dovah::sex::male].icons.message);
   this->ui.modelF->initializeFrom(working.world_models[dovah::sex::female].model);
   ui::bind(this->ui.iconInventoryF, working.world_models[dovah::sex::female].icons.inventory);
   ui::bind(this->ui.iconMessageF, working.world_models[dovah::sex::female].icons.message);
   ui::bind(this->ui.ragdollConstraintTemplate, working.ragdoll_constraint_template);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.soundTake, working.sounds.take, working);
   ui::bind(this->ui.soundDrop, working.sounds.drop, working);
   this->ui.armorAddons->pullStubs(working.armor_addons);
   this->ui.keywords->pullStubs(working.keywords.forms);
   this->ui.scriptListPane->setFormWorkingCopy(&working);
   this->_models.biped_objects->importFlags(working.biped_object);
}
void FormDialogArmor::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.name, this->ui.name->text());
   gls.assign_localized_string(working.description, this->ui.description->toPlainText());
   this->_models.biped_objects->exportFlags(working.biped_object);
   this->ui.modelM->commitTo(working.world_models[dovah::sex::male].model, working);
   this->ui.modelF->commitTo(working.world_models[dovah::sex::female].model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);
   this->ui.armorAddons->commitStubs(working.armor_addons, working);
   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.scriptListPane->commit();
}