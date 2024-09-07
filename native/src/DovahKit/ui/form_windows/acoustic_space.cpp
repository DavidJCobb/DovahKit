#include "./acoustic_space.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"

FormDialogAcousticSpace::FormDialogAcousticSpace(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   this->ui.envType->setAllowedFormType(dovah::form_type::reverb_parameters);
   this->ui.region->setAllowedFormType(dovah::form_type::region);
   this->ui.loopSound->setAllowedFormType(dovah::form_type::sound_descriptor);

   this->load(); // this creates the working copy.
}
void FormDialogAcousticSpace::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.envType, working.environment_type, working);
   ui::bind(this->ui.region, working.region, working);
   ui::bind(this->ui.loopSound, working.ambient_sound, working);
}
void FormDialogAcousticSpace::_save_impl() {
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