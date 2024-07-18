#include "./leveled_character.h"
#include "./leveled_lists/LeveledListEditDialogHelpers.h"

FormDialogLeveledCharacter::FormDialogLeveledCharacter(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   LeveledListEditDialogHelpers::setup(*this);

   this->load(); // this creates the working copy.
}

void FormDialogLeveledCharacter::_overwrite_selected_leveled_object() {
   #if _DEBUG
      auto* _widget_what_triggered_this_here_call = sender();
   #endif
   LeveledListEditDialogHelpers::write_ui_to_model(*this);
}

void FormDialogLeveledCharacter::_load_impl() {
   LeveledListEditDialogHelpers::load(*this);

   auto& working = *this->form;
   this->ui.model->initializeFrom(working.model);
}
void FormDialogLeveledCharacter::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   LeveledListEditDialogHelpers::save(*this);

   auto& working = *this->form;
   this->ui.model->commitTo(working.model, working);
}

/*virtual*/ bool FormDialogLeveledCharacter::eventFilter(QObject* watched, QEvent* event) /*override*/ {
   return LeveledListEditDialogHelpers::event_filter(*this, watched, event);
}