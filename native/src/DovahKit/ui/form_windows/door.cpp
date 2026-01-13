#include "./door.h"
#include "dovah/core.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"

FormDialogDoor::FormDialogDoor(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.soundOpen->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundClose->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.soundLoop->setAllowedFormType(dovah::form_type::sound_descriptor);
   this->ui.randomDestinations->setAllowedFormTypes({ dovah::form_type::cell, dovah::form_type::worldspace });

   this->load(); // this creates the working copy.
}
void FormDialogDoor::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.name));
   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   ui::bind(this->ui.soundOpen,  working.open_sound,  working);
   ui::bind(this->ui.soundClose, working.close_sound, working);
   ui::bind(this->ui.soundLoop,  working.loop_sound,  working);
   //
   {  // Flags
      auto& record_flags = this->record_flags();
      auto& unique_flags = working.door_flags;
      ui::bind(this->ui.flagRandomAnimStart, record_flags, loaded_form_type::form_flag::random_anim_start);
      ui::bind(this->ui.flagSlidingDoor,     unique_flags, loaded_form_type::door_flag::sliding);
      ui::bind(this->ui.flagAutomaticDoor,   unique_flags, loaded_form_type::door_flag::automatic);
      //
      ui::bind(this->ui.flagIsMarker,        record_flags, loaded_form_type::form_flag::is_marker);
      ui::bind(this->ui.flagHidden,          unique_flags, loaded_form_type::door_flag::hidden);
      ui::bind(this->ui.flagMinUse,          unique_flags, loaded_form_type::door_flag::minimal_use);
      //
      ui::bind(this->ui.flagNoCombatSearch,  unique_flags, loaded_form_type::door_flag::do_not_open_in_combat_search);
   }

   for (auto& ref : working.random_destinations) {
      this->ui.randomDestinations->addStub(ref.get_form_stub());
   }
   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogDoor::_save_impl() {
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

   this->ui.randomDestinations->commitStubs(working.random_destinations, working);
   this->ui.scriptListPane->commit();
}