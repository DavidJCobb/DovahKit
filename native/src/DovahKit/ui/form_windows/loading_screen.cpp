#include "./loading_screen.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogLoadingScreen::FormDialogLoadingScreen(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::set_unsigned_range<float>(this->ui.initialScale);
   this->ui.modelStatic->setAllowedFormType(dovah::form_type::statik);
   for (auto* widget : std::array{
      this->ui.initialPosX,
      this->ui.initialPosY,
      this->ui.initialPosZ,
   }) {
      widget->setRange(-1000, 1000);
   }
   for (auto* widget : std::array{
      this->ui.initialRotX,
      this->ui.initialRotY,
      this->ui.initialRotZ,
      this->ui.rotationMin,
      this->ui.rotationMax,
   }) {
      widget->setRange(-360, 360);
   }

   this->load(); // this creates the working copy.
}
void FormDialogLoadingScreen::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.description->setPlainText(gls.convert_localized_string(working.description));
   ui::bind(this->ui.cameraPath, working.camera_path);

   ui::bind(this->ui.flagShowsInMainMenu, this->record_flags(), loaded_form_type::form_flag::displays_in_main_menu);
   ui::bind(this->ui.modelStatic, working.static_model, working);
   ui::bind(this->ui.initialScale, working.initial_coords.scale);
   ui::bind(this->ui.initialPosX, working.initial_coords.translation.x);
   ui::bind(this->ui.initialPosY, working.initial_coords.translation.y);
   ui::bind(this->ui.initialPosZ, working.initial_coords.translation.z);
   ui::bind(this->ui.initialRotX, working.initial_coords.rotation.x);
   ui::bind(this->ui.initialRotY, working.initial_coords.rotation.y);
   ui::bind(this->ui.initialRotZ, working.initial_coords.rotation.z);
   ui::bind(this->ui.rotationMin, working.rotation_constraints.min);
   ui::bind(this->ui.rotationMax, working.rotation_constraints.max);

   this->ui.conditions->importFrom(working, working.conditions);
}
void FormDialogLoadingScreen::_save_impl() {
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
   
   gls.assign_localized_string(working.description, this->ui.description->toPlainText());

   this->ui.conditions->exportTo(working, working.conditions);
}