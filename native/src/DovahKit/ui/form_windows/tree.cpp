#include "./tree.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"

FormDialogTree::FormDialogTree(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.flexibilityBranch->setRange(0, 1000);
   this->ui.flexibilityTrunk->setRange(0, 1000);
   this->ui.leafAmplitude->setRange(0, 1000);
   this->ui.leafFrequency->setRange(0, 1000);
   //
   // NOTE: Other tree params are not exposed in the CK UI. I can't find 
   // where or how they're set.
   //
   
   this->ui.ingredient->setAllowedFormTypes({ dovah::form_type::ingredient, dovah::form_type::potion, form_type::leveled_item });
   this->ui.harvestSound->setAllowedFormType(dovah::form_type::sound_descriptor);

   this->load(); // this creates the working copy.
}
void FormDialogTree::_load_impl() {
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   this->ui.model->initializeFrom(working.model);
   ui::bind(this->ui.flexibilityTrunk, working.tree_data.trunk.flexibility);
   ui::bind(this->ui.flexibilityBranch, working.tree_data.branch_flexibility);
   ui::bind(this->ui.leafAmplitude, working.tree_data.leaf.amplitude);
   ui::bind(this->ui.leafFrequency, working.tree_data.leaf.frequency);
   ui::bind(this->ui.ingredient, working.harvestable.ingredient, working);
   ui::bind(this->ui.harvestSound, working.harvestable.harvest_sound, working);
   ui::bind(this->ui.harvestChanceSpring, working.harvestable.chance_by_season.spring);
   ui::bind(this->ui.harvestChanceSummer, working.harvestable.chance_by_season.summer);
   ui::bind(this->ui.harvestChanceAutumn, working.harvestable.chance_by_season.autumn);
   ui::bind(this->ui.harvestChanceWinter, working.harvestable.chance_by_season.winter);

   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogTree::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.name, this->ui.name->text());
   this->ui.model->commitTo(working.model, working);

   this->ui.scriptListPane->commit();
}