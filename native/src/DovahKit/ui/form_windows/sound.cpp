#include "./sound.h"
#include "ui/utils/bind.h"

FormDialogSound::FormDialogSound(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.soundDescriptor->setAllowedFormType(dovah::form_type::sound_descriptor);

   this->load(); // this creates the working copy.
}
void FormDialogSound::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.soundDescriptor, working.descriptor, working);
}
void FormDialogSound::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
}