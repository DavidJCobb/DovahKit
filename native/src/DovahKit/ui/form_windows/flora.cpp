#include "./flora.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"

FormDialogFlora::FormDialogFlora(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.ingredient->setAllowedFormTypes({ dovah::form_type::ingredient, dovah::form_type::potion, form_type::leveled_item });
   this->ui.harvestSound->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.keywords->setAllowedFormTypes({ dovah::form_type::keyword });

   QObject::connect(this->ui.ingredient, &DKFormPicker::formChanged, this, [this](dovah::form_stub* selected) {
      this->ui.harvestSound->setEnabled(selected != nullptr);
      this->ui.seasonalChances->setEnabled(selected != nullptr);
   });

   this->load(); // this creates the working copy.
}
void FormDialogFlora::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.ingredient,   working.harvestable.ingredient,    working);
   ui::bind(this->ui.harvestSound, working.harvestable.harvest_sound, working);
   ui::bind(this->ui.chanceSpring, working.harvestable.chance_by_season.spring);
   ui::bind(this->ui.chanceFall,   working.harvestable.chance_by_season.autumn);
   ui::bind(this->ui.chanceSummer, working.harvestable.chance_by_season.summer);
   ui::bind(this->ui.chanceWinter, working.harvestable.chance_by_season.winter);
   //
   this->ui.activateTextOverride->setText(gls.convert_localized_string(working.activation_verb));

   for (auto& ref : working.keywords.forms) {
      this->ui.keywords->addStub(ref.get_form_stub());
   }
   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogFlora::_save_impl() {
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
   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);
   gls.assign_localized_string(working.activation_verb, this->ui.activateTextOverride->text());

   this->ui.keywords->commitStubs(working.keywords.forms, working);
   this->ui.scriptListPane->commit();
}